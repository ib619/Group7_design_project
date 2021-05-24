import React, { useState, useEffect } from "react";
import { useSubscription } from "mqtt-react-hooks";
import styled from "styled-components";
import Level0 from "../assets/battery-level-0.svg";
import Level1 from "../assets/battery-level-1.svg";
import Level2 from "../assets/battery-level-2.svg";
import Level3 from "../assets/battery-level-3.svg";
import Level4 from "../assets/battery-level-4.svg";

// component to display current battery levels and health

const Battery = () => {
  const { message } = useSubscription("battery/status");
  const [battery, setBattery] = useState(60);

  useEffect(() => {
    if (message) {
      setBattery(JSON.parse(message.message));
    }
  }, [message, battery]);

  return (
    <>
      {battery <= 20 ? (
        <ImgContainer src={Level0} />
      ) : battery <= 40 ? (
        <ImgContainer src={Level1} />
      ) : battery <= 60 ? (
        <ImgContainer src={Level2} />
      ) : battery <= 80 ? (
        <ImgContainer src={Level3} />
      ) : (
        <ImgContainer src={Level4} />
      )}
    </>
  );
};

export default Battery;

const ImgContainer = styled.img`
  height: 45px;
  width: 45px;
  margin: 5px;
`;
