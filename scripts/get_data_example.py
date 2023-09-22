import os
import requests

# USAGE
# `SUPABASE_API_KEY="<api_key>" TABLE_NAME="<table_name>" python3 get_data_example.py

supabase_url = "https://nmkuqyrotfsszqglglld.supabase.co"
api_key = os.getenv("SUPABASE_API_KEY")
table_name = os.getenv("TABLE_NAME")

headers = {
    "apikey": api_key,
    "Content-Type": "application/json",
}

url = f"{supabase_url}/rest/v1/{table_name}"

response = requests.get(url, headers=headers)

if response.status_code == 201:
    print("Data got successfully")
    print(response.text)
else:
    print(f"Error getting data: {response.status_code} - {response.text}")
