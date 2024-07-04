import azure.functions as func
import logging
import csv
import io
import pyodbc
import os

def main(req: func.HttpRequest) -> func.HttpResponse:
    logging.info('Python HTTP trigger function processed a request.')

    # Check if a file is included in the request
    req_files = req.files.get('file')
    if not req_files:
        return func.HttpResponse(
            "Please upload a CSV file",
            status_code=400
        )

    try:
        # Read the CSV file
        csv_data = req_files[0].read().decode('utf-8')
        csv_reader = csv.reader(io.StringIO(csv_data), delimiter=',')

        # Connect to Azure SQL Database
        conn_str = (
            f"DRIVER={{ODBC Driver 18 for SQL Server}};"
            f"SERVER={os.environ['moneytrees-sqlserver.database.windows.net']};"
            f"DATABASE={os.environ['MoneyTreesDatabase']};"
            f"UID={os.environ['Flavia']};"
            f"PWD={os.environ['Darken&5h33p !']}"
        )

        conn = pyodbc.connect(conn_str)
        cursor = conn.cursor()

        # Skip the header row
        next(csv_reader, None)

        # Process each row in the CSV
        for row in csv_reader:
            user_id = int(row[0])
            company_id = int(row[1])
            shares_owned = int(row[2])

            # Insert into portfolios table
            cursor.execute("""
                INSERT INTO portfolios (user_id, company_id, shares_owned)
                VALUES (?, ?, ?)
            """, user_id, company_id, shares_owned)

        conn.commit()
        logging.info('CSV data uploaded and processed successfully.')

        return func.HttpResponse("CSV data uploaded successfully", status_code=200)

    except Exception as e:
        logging.error(f"Error processing CSV upload: {e}")
        return func.HttpResponse(
            "Internal server error occurred",
            status_code=500
        )

    finally:
        if 'cursor' in locals():
            cursor.close()
        if 'conn' in locals():
            conn.close()

