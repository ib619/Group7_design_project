import React from "react";
import Controller from "../components/Controller";
import Map from "../components/Map";

const ArrowPage = (props) => {
  return (
    <>
      <Map pos={props.pos} />
      <Controller data={props.pos} setData={props.setPos} />
    </>
  );
};

export default ArrowPage;
