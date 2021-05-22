import React, { useState, useEffect } from "react";
import Map from "../components/Map";
import { useMqttState } from "mqtt-react-hooks";

const InputPage = (props) => {
  // for the game
  const [pos, setPos] = useState({ x: 0, y: 0 });
  const [target, setTarget] = useState("");
  const { client } = useMqttState();

  const handleChange = (e) => {
    setTarget(e.target.value);
  };

  const handleSubmit = (e) => {
    e.preventDefault();
    console.log(target);
    setTarget("");
    client.publish("testingg", target);
  };

  return (
    <>
      <Map data={pos} />
      <form onSubmit={handleSubmit}>
        <label>
          Target Coordinates:
          <input
            type="text"
            name="target"
            value={target}
            onChange={handleChange}
          />
        </label>
        <input type="submit" />
      </form>
    </>
  );
};

export default InputPage;
