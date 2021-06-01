import React, { useState, useEffect } from "react";
import { useSubscription } from "mqtt-react-hooks";
import styled from "styled-components";
import driving from "../assets/driving.gif";

const RoverStatus = () => {
  const { message } = useSubscription("rover/status");
  const [status, setStatus] = useState({});

  useEffect(() => {
    if (message && message.topic === "rover/status") {
      setStatus(JSON.parse(message.message));
    }
  }, [message]);

  return (
    <React.Fragment>
      <img src={driving} alt="loading..." style={{ height: 50, width: 100 }} />
    </React.Fragment>
  );
};

export default RoverStatus;

// const gifContainer = styled.img`
//   height: 50px;
//   width: 50px;
//   margin: 5px;
// `;
