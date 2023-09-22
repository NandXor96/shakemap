import Map, {
  CircleLayer,
  HeatmapLayer,
  Layer,
  Source,
} from "react-map-gl/maplibre";
import "maplibre-gl/dist/maplibre-gl.css";

import type { FeatureCollection } from "geojson";
import { useEffect, useState } from "react";

const circleLayer: CircleLayer = {
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
      "#D64550",
      100,
      "#475B63",
    ],
  },
};

const MAX_HM_ZOOM = 18;
const heatmapLayer: HeatmapLayer = {
  id: "earthquakes-heat",
  type: "heatmap",
  source: "point",
  maxzoom: MAX_HM_ZOOM,
  paint: {
    "heatmap-weight": [
      "interpolate",
      ["linear"],
      ["get", "value"],
      0,
      0,
      100,
      10,
    ],
    "heatmap-intensity": [
      "interpolate",
      ["linear"],
      ["zoom"],
      MAX_HM_ZOOM - 10,
      1,
      MAX_HM_ZOOM,
      1,
    ],
    // Color ramp for heatmap.  Domain is 0 (low) to 1 (high).
    // Begin color ramp at 0-stop with a 0-transparancy color
    // to create a blur-like effect.
    "heatmap-color": [
      "interpolate",
      ["linear"],
      ["heatmap-density"],
      0,
      "rgba(0,0,0,0)",
      0.1,
      "#DAEFB3",
      0.5,
      "#F09D51",
      1,
      "#D64550",
    ],
    "heatmap-radius": [
      "interpolate",
      ["linear"],
      ["zoom"],
      0,
      2,
      MAX_HM_ZOOM,
      25,
    ],
    "heatmap-opacity": [
      "interpolate",
      ["linear"],
      ["zoom"],
      MAX_HM_ZOOM - 3,
      1,
      MAX_HM_ZOOM,
      0,
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
          <Layer {...circleLayer} />
          <Layer {...heatmapLayer} />
        </Source>
      </Map>
    </>
  );
}

export default App;
