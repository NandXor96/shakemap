import Map from "react-map-gl/maplibre";
import "maplibre-gl/dist/maplibre-gl.css";

import type { FeatureCollection } from "geojson";
import { useEffect, useState } from "react";
import { Points } from "./Points";
import { Lines } from "./Lines";

async function getData(url: string) {
  const API_KEY =
    "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im5ta3VxeXJvdGZzc3pxZ2xnbGxkIiwicm9sZSI6ImFub24iLCJpYXQiOjE2OTUzNzkyNjIsImV4cCI6MjAxMDk1NTI2Mn0.elRWzQwl_lppri_s87Ho2sfd0HLodYKzCAQVpNXNgwA";

  const result = await window.fetch(url, {
    headers: {
      apikey: API_KEY,
      Authorization: `Bearer ${API_KEY}`,
    },
  });
  const [{ json_build_object: data }]: {
    json_build_object: FeatureCollection;
  }[] = await result.json();
  return data;
}

function App() {
  const [msHackMode, setMsHackMode] = useState<boolean>(false);
  const [pointData, setPointData] = useState<FeatureCollection>();
  const [lineData, setLineData] = useState<FeatureCollection>();

  async function updateData() {
    const pointUrl =
      "https://nmkuqyrotfsszqglglld.supabase.co/rest/v1/points_mapped?";
    const lineUrl =
      "https://nmkuqyrotfsszqglglld.supabase.co/rest/v1/segments_mapped?";

    const pointData = await getData(pointUrl);
    const lineData = await getData(lineUrl);

    console.log(lineData);

    setPointData(pointData);
    setLineData(lineData);
  }
  useEffect(() => {
    updateData();
  }, []);

  return (
    <div className={`app ${msHackMode ? "mshack" : "regular"}`}>
      <div className="button" onClick={() => setMsHackMode((m) => !m)}>
        MS-Hack
      </div>
      <Map
        initialViewState={{
          longitude: 7.6261,
          latitude: 51.9607,
          zoom: 14,
        }}
        style={{ width: "100%", height: "100%" }}
        mapStyle={
          msHackMode
            ? "https://maps.moritz.tk/style-ms-hack.json"
            : "https://maps.moritz.tk/style.json"
        }
      >
        <Points data={pointData} />
        <Lines data={lineData} />
      </Map>
    </div>
  );
}

export default App;
