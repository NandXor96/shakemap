import requests
import os

# Define your Supabase URL and API key
supabase_url = "https://nmkuqyrotfsszqglglld.supabase.co"
api_key = os.environ.get("SUPABASE_PRIVATE_API_KEY")

# Define the endpoint for your table
table_name = "measurements_v2"

# Define the data you want to post (replace with your own data)
data = [
    {
        "location": "SRID=4326;POINT(7.623087189150482 51.95714830996306)",  # Assuming location is of type geography
        "sensor_id": "supabase-rest.py",
        "value": int(3),
        "timestamp": "2023-09-22T12:00:00Z"  # Assuming timestamp is in ISO 8601 format
    },
    {
        "location": "SRID=4326;POINT(7.6238589032288075 51.95698032812666)",  # Assuming location is of type geography
        "sensor_id": "supabase-rest.py",
        "value": int(2),
        "timestamp": "2023-09-22T13:00:00Z"  # Assuming timestamp is in ISO 8601 format
    }
]

# Create the headers with the API key
headers = {
    "apikey": api_key,
    "Content-Type": "application/json",
}

# Create the URL for the specific table
url = f"{supabase_url}/rest/v1/{table_name}"

# Make the POST request
response = requests.post(url, headers=headers, json=data)

# Check if the request was successful (status code 201)
if response.status_code == 201:
    print("Data posted successfully")
else:
    print(f"Error posting data: {response.status_code} - {response.text}")
