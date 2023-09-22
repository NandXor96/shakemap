import os
import json
import tqdm.contrib
import random
import time
from supabase import create_client, Client

url: str = "https://nmkuqyrotfsszqglglld.supabase.co"
key: str = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im5ta3VxeXJvdGZzc3pxZ2xnbGxkIiwicm9sZSI6ImFub24iLCJpYXQiOjE2OTUzNzkyNjIsImV4cCI6MjAxMDk1NTI2Mn0.elRWzQwl_lppri_s87Ho2sfd0HLodYKzCAQVpNXNgwA"
supabase: Client = create_client(url, key)


i = 0
while i < 1000:
    i += 1
    x = random.uniform(7.604875810227526,7.640794983586773)
    y = random.uniform(51.97353578245037, 51.94897760105292)
    payload = {
        #"id": i+3,
        "location": f"POINT({x} {y})",
        "mean_0_1hz": random.uniform(10, 20),
        "stdev_0_1hz": random.uniform(10, 20),
        "mean_1_1hz": random.uniform(10, 20),
        "stdev_1_1hz": random.uniform(10, 20),
        "mean_1_2hz": random.uniform(1, 20),
        "stdev_1_2hz": random.uniform(1, 20),
        "mean_1_3hz": random.uniform(1, 10),
        "stdev_1_3hz": random.uniform(1, 10),
        "mean_1_4hz": random.uniform(1, 10),
        "stdev_1_4hz": random.uniform(1, 10),
        "mean_1_5hz": random.uniform(1, 10),
        "stdev_1_5hz": random.uniform(1, 10),
    }
    
    supabase.table('ds_measurements').insert(payload).execute()
    time.sleep(1)

