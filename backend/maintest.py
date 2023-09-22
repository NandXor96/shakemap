import os
import json
import tqdm.contrib
from supabase import create_client, Client

url: str = "https://nmkuqyrotfsszqglglld.supabase.co"
key: str = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im5ta3VxeXJvdGZzc3pxZ2xnbGxkIiwicm9sZSI6ImFub24iLCJpYXQiOjE2OTUzNzkyNjIsImV4cCI6MjAxMDk1NTI2Mn0.elRWzQwl_lppri_s87Ho2sfd0HLodYKzCAQVpNXNgwA"
supabase: Client = create_client(url, key)

with open('export.geojson') as file:
    content = json.load(file)

    for i, feat in tqdm.contrib.tenumerate(content['features']):
        
        payload =         {
            "id": i,
            "feature_json": feat
        }
        
        supabase.table('osm').insert(payload).execute()

