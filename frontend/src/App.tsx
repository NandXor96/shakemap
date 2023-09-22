import Map from "react-map-gl/maplibre";
import "maplibre-gl/dist/maplibre-gl.css";

function App() {
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
      />
    </>
  );
}

export default App;
