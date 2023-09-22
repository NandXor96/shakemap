import { useRef, useEffect, useState } from "react";
import maplibregl from "maplibre-gl";
import "maplibre-gl/dist/maplibre-gl.css";

import styles from "./Map.module.css";

export function Map() {
  const mapContainer = useRef(null);
  const map = useRef<maplibregl.Map | null>(null);
  const [lng] = useState(7.6261);
  const [lat] = useState(51.9607);
  const [zoom] = useState(14);

  useEffect(() => {
    if (map.current) return; // stops map from intializing more than once
    if (!mapContainer.current) return;

    map.current = new maplibregl.Map({
      container: mapContainer.current,
      style: `https://maps.moritz.tk/style.json`,
      center: [lng, lat],
      zoom: zoom,
    });
  }, [lng, lat, zoom]);

  return (
    <div className={styles.wrapper}>
      <div ref={mapContainer} className={styles.map} />
    </div>
  );
}
