import os
import requests

# USAGE
# `SUPABASE_API_KEY="<api_key>" TABLE_NAME="<table_name>" python3 post_data_example.py

# Define your Supabase URL and API key
supabase_url = "https://nmkuqyrotfsszqglglld.supabase.co"
api_key = os.getenv("SUPABASE_API_KEY")
table_name = os.getenv("TABLE_NAME")

data = [
    {
        # Assuming location is of type geography
        "location": "POINT(-77.0369 38.8951)",
        "sensor_type": "initial",
        # Assuming timestamp is in ISO 8601 format
        "timestamp": "2023-09-22T12:00:00Z",
        "mean_0_1hz": 0.701,
        "stdev_0_1hz": 0.150,
        "mean_1_1hz": 0.701,
        "stdev_1_1hz": 0.150,
        "mean_1_2hz": 0.701,
        "stdev_1_2hz": 0.150,
        "mean_1_3hz": 0.701,
        "stdev_1_3hz": 0.150,
        "mean_1_4hz": 0.701,
        "stdev_1_4hz": 0.150,
        "mean_1_5hz": 0.701,
        "stdev_1_5hz": 0.150,
        "mean_1_6hz": 0.701,
        "stdev_1_6hz": 0.150,
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
