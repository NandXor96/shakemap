import Map, { CircleLayer, Layer, Source } from "react-map-gl/maplibre";
import "maplibre-gl/dist/maplibre-gl.css";

import type { FeatureCollection } from "geojson";
import { useEffect, useState } from "react";

const layerStyle: CircleLayer = {
  id: "point",
  source: "point",
  type: "circle",
  paint: {
    "circle-radius": 10,
    "circle-color": [
      "interpolate",
      ["linear"],
      ["get", "value"],
      0,
      "#475B63",
      100,
      "#D64550",
    ],
  },
};

function App() {
  const [geojson, setGeojson] = useState<FeatureCollection>();
  async function updateData() {
    const url =
      "https://nmkuqyrotfsszqglglld.supabase.co/rest/v1/heatmap_view?";
    const API_KEY =
      "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im5ta3VxeXJvdGZzc3pxZ2xnbGxkIiwicm9sZSI6ImFub24iLCJpYXQiOjE2OTUzNzkyNjIsImV4cCI6MjAxMDk1NTI2Mn0.elRWzQwl_lppri_s87Ho2sfd0HLodYKzCAQVpNXNgwA";

    const result = await window.fetch(url, {
      headers: {
        apikey: API_KEY,
        Authorization: `Bearer ${API_KEY}`,
      },
    });
    const [{ json_build_object: geojson }]: {
      json_build_object: FeatureCollection;
    }[] = await result.json();
    console.log(geojson);
    setGeojson(geojson);
  }
  useEffect(() => {
    updateData();
  }, []);

  return (
    <>
      <Map
        initialViewState={{
          longitude: 7.6261,
          latitude: 51.9607,
          zoom: 14,
        }}
        style={{ width: "100%", height: "100%" }}
        mapStyle="https://maps.moritz.tk/style.json"
      >
        <Source id="point" type="geojson" data={geojson}>
          <Layer {...layerStyle} />
        </Source>
      </Map>
    </>
  );
}

export default App;
