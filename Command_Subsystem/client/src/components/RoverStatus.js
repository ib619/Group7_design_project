import React, { useState, useEffect } from "react";
import { useMqttState, useSubscription } from "mqtt-react-hooks";
import { Button } from "react-bootstrap";
import styled from "styled-components";

const RoverStatus = () => {
  const { connectionStatus, client } = useMqttState();
  const { message } = useSubscription("rover/status");
  const [status, setStatus] = useState({
    drive_status: 2,
    range: 0,
    obstacle_detected: 0,
  });

  useEffect(() => {
    if (message && message.topic === "rover/status") {
      let data = JSON.parse(message.message);
      setStatus(data);
    }
  }, [message]);

  useEffect(() => {
    if (status["drive_status"] === 2) {
      localStorage.removeItem("obstacles");
    }
  });

  const handleClick = (e) => {
    client.publish("reset", e.target.name);
  };

  const mapStatus = () => {
    let mapping = {
      0: "Rover is chilling!",
      1: "Rover is driving!",
      2: "Rover is at home!",
    };

    return mapping[status["drive_status"]];
  };

  return (
    <Container>
      <p>MQTT: {connectionStatus}</p>
      {/* <img src={driving} alt="loading..." style={{ height: 50, width: 100 }} /> */}
      <p>{mapStatus()}</p>
      <p>Remaining Range: {status["range"]} cm</p>
      <Button name={1} variant="outline-light" onClick={handleClick}>
        Reset
      </Button>
      <Button name={2} variant="outline-danger" onClick={handleClick}>
        Stop
      </Button>
    </Container>
  );
};

export default RoverStatus;

const Container = styled.div`
  display: flex;
  margin: 1rem;
  padding: 0.5rem;
  justify-content: space-between;

  .p {
    color: ${({ theme }) => theme.text};
    text-align: center;
    transition: all 0.5s ease-out;
  }
`;
