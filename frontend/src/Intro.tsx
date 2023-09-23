import styles from "./Intro.module.css";
export function Intro() {
  return (
    <div className={styles.intro}>
      <h1>Willkommen</h1>
      Wähle ein farbiges Segment um Informationen zum Zustand des Fahrradweges
      an dieser Stelle anzuzeigen.
    </div>
  );
}
