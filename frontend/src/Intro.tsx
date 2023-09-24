import styles from "./Intro.module.css";
export function Intro() {
  return (
    <div className={styles.intro}>
      <h1>Willkommen</h1>
      Die ShakeMap zeigt den Zustand der Radwege in Münster an. 
      Wähle ein farbiges (rotes) Segment, um Informationen zum Zustand des Fahrradweges
      an dieser Stelle anzuzeigen.
      Weitere Informationen findest du in unserer 
      <a href="https://docs.google.com/presentation/d/1HRrx9XLUtCg5AqGWX6iDJYRp8W0XrIjcx5gh5JsdXoM/edit?usp=sharing">Präsentation</a>
      oder in unserem <a href="https://www.youtube.com/live/K8P3D5k8daE?si=hnmN-PzFpB2raFCw&t=6698">Picht</a>.
    </div>
  );
}
