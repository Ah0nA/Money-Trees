import azure.functions as func
import logging
import os
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText

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
app = func.FunctionApp(http_auth_level=func.AuthLevel.ANONYMOUS)

@app.route(route="EmailNotification")
def EmailNotification(req: func.HttpRequest) -> func.HttpResponse:
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
