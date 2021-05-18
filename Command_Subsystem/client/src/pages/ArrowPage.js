import React, { useState } from "react";
import Controller from "../components/Controller";
import Map from "../components/Map";

const ArrowPage = () => {
  // for the game
  const [pos, setPos] = useState({ x: 0, y: 0 });

  return (
    <>
      <Map data={pos} />
      <Controller data={pos} setData={setPos} />
    </>
  );
};

export default ArrowPage;
