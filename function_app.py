import datetime
import logging
import os
import csv
import io
import pyodbc
import http.client
import urllib.parse
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
import azure.functions as func

app = func.FunctionApp()

@app.timer_trigger(schedule="0 */5 * * * *", name="myTimer", run_on_startup=True)
def fetch_esg_data(timer: func.TimerRequest) -> None:
    if timer.past_due:
        logging.info('The timer is past due!')

    logging.info('Python timer trigger function executed.')

    # Connect to Azure SQL Database
    conn_str = (
        f"DRIVER={{ODBC Driver 18 for SQL Server}};"
        f"SERVER={os.environ['moneytrees-sqlserver.database.windows.net']};"
        f"DATABASE={os.environ['MoneyTreesDatabase']};"
        f"UID={os.environ['Flavia']};"
        f"PWD={os.environ['Darken&5h33p !']}"
    )

    try:
        conn = pyodbc.connect(conn_str)
        cursor = conn.cursor()

        # Get current user's email (replace with your actual method to get user email)
        user_email = get_current_user_email()

        # Fetch companies from portfolios table for the current user
        cursor.execute("""
            SELECT DISTINCT p.company_id, p.company_name 
            FROM portfolios p 
            JOIN users u ON p.user_id = u.user_id 
            WHERE u.email = ?
        """, user_email)
        companies = cursor.fetchall()

        for company_id, company_name in companies:
            # Fetch ESG data from external API
            esg_data = fetch_esg_score(company_name)

            if esg_data is not None:
                process_esg_data(cursor, company_id, company_name, esg_data)
            else:
                logging.warning(f"ESG data not found for {company_name}. Skipping.")

        conn.commit()
        logging.info('ESG data updated successfully.')

    except pyodbc.Error as e:
        logging.error(f"Database error: {e}")

    finally:
        if 'cursor' in locals():
            cursor.close()
        if 'conn' in locals():
            conn.close()

    logging.info('Python timer trigger function completed.')

def get_current_user_email():
    # Replace with your actual logic to get the current user's email (from session, JWT, etc.)
    # For testing purposes, return a hardcoded email
    return "example@example.com"

def fetch_esg_score(company_name):
    try:
        conn = http.client.HTTPSConnection("gaialens-esg-scores.p.rapidapi.com")
        headers = {
            'x-rapidapi-key': os.environ['fb3cfa337cmsh9505744be93fce7p1f31f3jsnd04180094d8c'],
            'x-rapidapi-host': "gaialens-esg-scores.p.rapidapi.com"
        }
        url = f"/scores?companyname={urllib.parse.quote(company_name)}"
        conn.request("GET", url, headers=headers)
        response = conn.getresponse()
        data = response.read()
        esg_data = data.decode("utf-8")
        conn.close()
        return esg_data

    except Exception as e:
        logging.error(f"Error fetching ESG data for {company_name}: {e}")
        return None

def process_esg_data(cursor, company_id, company_name, esg_data):
    esg_score = float(esg_data)
    stock_price = fetch_stock_price(company_name)  # Assuming a function to fetch stock price
    updated_at = datetime.datetime.utcnow()

    # Check if the company exists in esg_data table
    cursor.execute("SELECT 1 FROM esg_data WHERE company_id = ?", company_id)
    if cursor.fetchone():
        # Update existing record
        cursor.execute("""
            UPDATE esg_data
            SET esg_score = ?, updated_at = ?
            WHERE company_id = ?
        """, esg_score, updated_at, company_id)
    else:
        # Insert new record
        cursor.execute("""
            INSERT INTO esg_data (company_id, company_name, esg_score, updated_at)
            VALUES (?, ?, ?, ?)
        """, company_id, company_name, esg_score, updated_at)

    # Insert into historical_data table
    cursor.execute("""
        INSERT INTO historical_data (company_id, company_name, esg_score, stock_price, date)
        VALUES (?, ?, ?, ?, ?)
    """, company_id, company_name, esg_score, stock_price, updated_at)

def fetch_stock_price(company_name):
    # Implement a function to fetch stock price from a suitable API or source
    # Example: Using Yahoo Finance API as per your provided code
    try:
        conn = http.client.HTTPSConnection("yahoo-finance160.p.rapidapi.com")
        headers = {
            'x-rapidapi-key': os.environ['fb3cfa337cmsh9505744be93fce7p1f31f3jsnd04180094d8c'],
            'x-rapidapi-host': "yahoo-finance160.p.rapidapi.com",
            'Content-Type': "application/json"
        }
        payload = f"{{\"messages\":[{{\"role\":\"user\",\"content\":\"should i buy {company_name}\"}}],\"stock\":\"{company_name}\",\"conversation_id\":\"\",\"period\":\"1mo\"}}"
        conn.request("POST", "/finbot", payload, headers)
        res = conn.getresponse()
        data = res.read()
        stock_data = data.decode("utf-8")
        conn.close()
        return stock_data

    except Exception as e:
        logging.error(f"Error fetching stock price for {company_name}: {e}")
        return None


@app.route(route="csv_upload", auth_level=func.AuthLevel.Anonymous)
def csv_upload(req: func.HttpRequest) -> func.HttpResponse:
   

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



def send_email(smtp_server, smtp_port, smtp_user, smtp_password, subject, body, to_email):
    msg = MIMEMultipart()
    msg['From'] = smtp_user
    msg['To'] = to_email
    msg['Subject'] = subject
    msg.attach(MIMEText(body, 'plain'))

    try:
        server = smtplib.SMTP(smtp_server, smtp_port)
        server.starttls()
        server.login(smtp_user, smtp_password)
        server.send_message(msg)
        server.quit()
        logging.info(f"Email sent to {to_email}")
    except Exception as e:
        logging.error(f"Failed to send email: {str(e)}")

@app.route(route="email_notifications", auth_level=func.AuthLevel.ANONYMOUS)
def email_notifications(req: func.HttpRequest) -> func.HttpResponse:
    logging.info('Python HTTP trigger function processed a request.')

    try:
        req_body = req.get_json()
    except ValueError:
        return func.HttpResponse(
            "Invalid request body. Expected JSON.",
            status_code=400
        )

    smtp_server = os.getenv('SMTP_SERVER')
    smtp_port = os.getenv('SMTP_PORT')
    smtp_user = os.getenv('SMTP_USER')
    smtp_password = os.getenv('SMTP_PASSWORD')

    if not all([smtp_server, smtp_port, smtp_user, smtp_password]):
        return func.HttpResponse(
            "Missing one or more required environment variables: SMTP_SERVER, SMTP_PORT, SMTP_USER, SMTP_PASSWORD.",
            status_code=500
        )

    user_email = req_body.get('email')
    company = req_body.get('company')
    old_score = req_body.get('old_score')
    new_score = req_body.get('new_score')

    if not all([user_email, company, old_score, new_score]):
        return func.HttpResponse(
            "Missing one or more required fields: email, company, old_score, new_score.",
            status_code=400
        )

    subject = f"ESG Score Update for {company}"
    body = f"The ESG score for {company} has changed from {old_score} to {new_score}."

    send_email(smtp_server, smtp_port, smtp_user, smtp_password, subject, body, user_email)

    return func.HttpResponse(
        "Email notification sent successfully.",
        status_code=200
    )
