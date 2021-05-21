import React, { useState, useEffect } from "react";
import Map from "../components/Map";
import axios from "axios";

const InputPage = () => {
  // for the game
  const [pos, setPos] = useState({ x: 0, y: 0 });

  const getPosition = () => {
    axios
      .get("/obstacles")
      .then((res) => setPos(res.data[0]))
      .catch((err) => console.log(err));
  };

  useEffect(() => {
    getPosition();
  }, []);

  return (
    <>
      <Map data={pos} />
      <form>
        <label>
          Target Coordinates:
          <input type="text" name="name" />
        </label>
        <button type="submit">Add</button>
      </form>
    </>
  );
};

export default InputPage;
