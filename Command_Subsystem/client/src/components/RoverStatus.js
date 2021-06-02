import React, { useState, useEffect } from "react";
import { useSubscription } from "mqtt-react-hooks";
import styled from "styled-components";
import driving from "../assets/driving.gif";

const RoverStatus = () => {
  const { message } = useSubscription("rover/status");
  const [drive, setDrive] = useState(2);
  const [range, setRange] = useState(0);

  useEffect(() => {
    if (message && message.topic === "rover/status") {
      let data = JSON.parse(message.message);
      setDrive(data["drive_status"]);
      setRange(data["range"]);
    }
  }, [message]);

  useEffect(() => {
    if (drive === 2) {
      localStorage.clear("position");
      localStorage.clear("obstacles");
    }
  });

  return (
    <React.Fragment>
      <img src={driving} alt="loading..." style={{ height: 50, width: 100 }} />
      <span>Remaining Range: {range}</span>
    </React.Fragment>
  );
};

export default RoverStatus;

// const gifContainer = styled.img`
//   height: 50px;
//   width: 50px;
//   margin: 5px;
// `;
