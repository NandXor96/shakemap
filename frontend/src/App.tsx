import Map, {
  MapLayerMouseEvent,
  MapRef,
  Point,
  PointLike,
} from "react-map-gl/maplibre";
import "maplibre-gl/dist/maplibre-gl.css";

import type { FeatureCollection } from "geojson";
import { useEffect, useRef, useState } from "react";
import { Points } from "./Points";
import { Lines } from "./Lines";

async function getData(url: string) {
  const API_KEY =
    "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im5ta3VxeXJvdGZzc3pxZ2xnbGxkIiwicm9sZSI6ImFub24iLCJpYXQiOjE2OTU0NjUxNTAsImV4cCI6MjAxMTA0MTE1MH0.I0MK5GFehsJ0e9riAcb_NtdiFhWFKSUmC05lxWW3npA";

  const result = await window.fetch(url, {
    headers: {
      apikey: API_KEY,
    },
  });
  const [{ json_build_object: data }]: {
    json_build_object: FeatureCollection;
  }[] = await result.json();
  return data;
}

function App() {
  const mapRef = useRef<MapRef>(null);
  const [msHackMode, setMsHackMode] = useState<boolean>(false);
  const [pointData, setPointData] = useState<FeatureCollection>();
  const [lineData, setLineData] = useState<FeatureCollection>();

  const [highlightedLine, setHighlightedLine] = useState("none");

  async function updateData() {
    const pointUrl =
      "https://nmkuqyrotfsszqglglld.supabase.co/rest/v1/points_v2";
    const lineUrl =
      "https://nmkuqyrotfsszqglglld.supabase.co/rest/v1/segments_mapped_v2";

    const pointData = await getData(pointUrl);
    const lineData = await getData(lineUrl);

    console.log(lineData);

    setPointData(pointData);
    setLineData(lineData);
  }
  useEffect(() => {
    updateData();
  }, []);

  function handleClick(event: MapLayerMouseEvent) {
    console.log(event);
    console.log(event.features);

    const map = mapRef.current;
    if (!map) return;

    const selectedLines = map.queryRenderedFeatures(getBbox(event.point, 10), {
      layers: ["line-segment-layer"],
    });

    const lineId = selectedLines[0]?.properties.line_id;

    setHighlightedLine(lineId || "none");
  }

  return (
    <div className={`app ${msHackMode ? "mshack" : "regular"}`}>
      <nav>
        <div className="button" onClick={() => setMsHackMode((m) => !m)}>
          MS-Hack
        </div>
      </nav>
      <Map
        ref={mapRef}
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
        onClick={handleClick}
      >
        <Points
          id="data-point-layer"
          highlightedLine={highlightedLine || "none"}
          data={pointData}
        />
        {<Lines id="line-segment-layer" data={lineData} />}
      </Map>
    </div>
  );
}

export default App;

function getBbox(point: Point, offset: number) {
  return [
    [point.x - offset, point.y - offset],
    [point.x + offset, point.y + offset],
  ] as [PointLike, PointLike];
}
