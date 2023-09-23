import Map, {
  MapGeoJSONFeature,
  MapLayerMouseEvent,
  MapRef,
  Point,
  PointLike,
} from "react-map-gl/maplibre";
import "maplibre-gl/dist/maplibre-gl.css";

import LogoMono from "./assets/logo_mono.svg?react";
import LogoColor from "./assets/logo_color.svg?react";

import type { FeatureCollection } from "geojson";
import { useEffect, useRef, useState } from "react";
import { Points } from "./Points";
import { Lines } from "./Lines";
import { LineInfo } from "./LineInfo";
import { Lanes } from "./Lanes";
import { Intro } from "./Intro";

async function getData(url: string) {
  const API_KEY =
    "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im5ta3VxeXJvdGZzc3pxZ2xnbGxkIiwicm9sZSI6ImFub24iLCJpYXQiOjE2OTU0NjUxNTAsImV4cCI6MjAxMTA0MTE1MH0.I0MK5GFehsJ0e9riAcb_NtdiFhWFKSUmC05lxWW3npA";

  const result = await window.fetch(url, {
    headers: {
      apikey: API_KEY,
    },
  });
  const data: {
    json_build_object: FeatureCollection;
  }[] = await result.json();

  return data[0]?.json_build_object;
}

function App() {
  const mapRef = useRef<MapRef>(null);
  const [msHackMode, setMsHackMode] = useState<boolean>(false);
  const [laneData, setLaneData] = useState<FeatureCollection>();
  const [pointData, setPointData] = useState<FeatureCollection>();
  const [lineData, setLineData] = useState<FeatureCollection>();
  const [closed, setClosed] = useState(false);

  const [highlightedLine, setHighlightedLine] = useState<MapGeoJSONFeature>();

  async function updateData() {
    const laneUrl = "https://nmkuqyrotfsszqglglld.supabase.co/rest/v1/osm_geom";
    const pointUrl =
      "https://nmkuqyrotfsszqglglld.supabase.co/rest/v1/points_v2";
    const lineUrl =
      "https://nmkuqyrotfsszqglglld.supabase.co/rest/v1/segments_mapped_v3";

    const laneData = await getData(laneUrl);
    const pointData = await getData(pointUrl);
    const lineData = await getData(lineUrl);

    console.log(lineData);

    setLaneData(laneData);
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

    setHighlightedLine(selectedLines[0]);
    if (selectedLines[0]) {
      setClosed(false);
    }
  }

  return (
    <div className={`app ${msHackMode ? "mshack" : "regular"}`}>
      <Map
        ref={mapRef}
        initialViewState={{
          longitude: 7.64016,
          latitude: 51.95169,
          zoom: 14,
        }}
        style={{ position: "absolute", width: "100%", height: "100%" }}
        mapStyle={
          msHackMode
            ? "https://maps.moritz.tk/style-ms-hack.json"
            : "https://maps.moritz.tk/style.json"
        }
        onClick={handleClick}
      >
        {laneData && <Lanes data={laneData} />}
        {lineData && <Lines id="line-segment-layer" data={lineData} />}
        {pointData && (
          <Points
            id="data-point-layer"
            highlightedLine={highlightedLine?.properties.line_id || "none"}
            data={pointData}
          />
        )}
      </Map>
      <div className="button mapstyle" onClick={() => setMsHackMode((m) => !m)}>
        Hack-Mode: {msHackMode ? "on" : "off"}
      </div>
      {closed ? null : (
        <aside>
          <div className="header">
            <div className="title">ShakeMap</div>
            {msHackMode ? (
              <LogoColor className="logo" />
            ) : (
              <LogoMono className="logo" />
            )}
          </div>
          {highlightedLine ? <LineInfo line={highlightedLine} /> : <Intro />}
          <div className="button" onClick={() => setClosed(true)}>
            Schließen
          </div>
        </aside>
      )}
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
