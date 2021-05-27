import React, { useState, useEffect } from "react";
import styled from "styled-components";
import { useSubscription } from "mqtt-react-hooks";
import LevelDead from "../assets/signal-level-dead.svg";
import Level0 from "../assets/signal-level-0.svg";
import Level1 from "../assets/signal-level-1.svg";
import Level2 from "../assets/signal-level-2.svg";
import Level3 from "../assets/signal-level-3.svg";

const SignalStrength = () => {
  const { message } = useSubscription("rssi");
  const [strength, setStrength] = useState(-20);

  useEffect(() => {
    if (message && message.topic === "rssi") {
      setStrength(JSON.parse(message.message));
    }
  }, [message, strength]);

  return (
    <>
      {strength <= -80 ? (
        <ImgContainer src={LevelDead} />
      ) : strength <= -70 ? (
        <ImgContainer src={Level0} />
      ) : strength <= -60 ? (
        <ImgContainer src={Level1} />
      ) : strength <= -30 ? (
        <ImgContainer src={Level2} />
      ) : (
        <ImgContainer src={Level3} />
      )}
    </>
  );
};

export default SignalStrength;

const ImgContainer = styled.img`
  height: 45px;
  width: 45px;
  margin: 5px;
  transition: all 0.6s ease-out;
`;
