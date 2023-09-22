import Map, { Layer, LineLayer, Source } from "react-map-gl/maplibre";
import "maplibre-gl/dist/maplibre-gl.css";

import type { FeatureCollection, Feature } from "geojson";
import { useEffect, useState } from "react";
import { Heatmap } from "./Heatmap";

const fakeData: FeatureCollection = {
  type: "FeatureCollection",
  features: [
    {
      type: "Feature",
      properties: {
        "@id": "way/4863787",
        bicycle: "designated",
        foot: "designated",
        highway: "path",
        lit: "no",
        oneway: "yes",
        path: "sidewalk",
        segregated: "no",
        surface: "asphalt",
      },
      geometry: {
        type: "LineString",
        coordinates: [
          [7.6595735, 51.9847142],
          [7.6596257, 51.9847857],
          [7.6595904, 51.9848718],
          [7.6593508, 51.9851082],
          [7.6591775, 51.9852728],
          [7.6590841, 51.9853832],
          [7.6590135, 51.9856014],
          [7.6589419, 51.9857328],
          [7.658941, 51.9858043],
          [7.6589945, 51.9858944],
          [7.659094, 51.9859888],
          [7.6592469, 51.9860886],
          [7.6594262, 51.9861552],
          [7.6595509, 51.9861929],
          [7.6596684, 51.9862142],
          [7.6597842, 51.9862326],
        ],
      },
      id: "way/4863787",
    },
  ],
};

const lineLayer: LineLayer = {
  type: "line",
  source: "line",
  id: "line",
  paint: {
    "line-color": "red",
    "line-width": 14,
    // 'line-gradient' must be specified using an expression
    // with the special 'line-progress' property
    "line-gradient": [
      "interpolate",
      ["linear"],
      ["line-progress"],
      0,
      "transparent",
      0.35,
      "transparent",
      0.39,
      "#D64550",
      0.4,
      "#D64550",
      0.41,
      "#D64550",
      0.45,
      "transparent",
      1,
      "transparent",
    ],
  },
  layout: {
    "line-cap": "round",
    "line-join": "round",
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
        <Heatmap data={geojson} />
        <Source id="line" type="geojson" data={fakeData} lineMetrics>
          <Layer {...lineLayer} />
        </Source>
      </Map>
    </>
  );
}

export default App;
