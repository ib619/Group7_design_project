import React, { useState, useEffect } from "react";
import { useMqttState, useSubscription } from "mqtt-react-hooks";
import { Button } from "react-bootstrap";
import styled from "styled-components";

const Status = () => {
  const { connectionStatus, client } = useMqttState();
  const { message } = useSubscription("rover/status");
  const [drive, setDrive] = useState(0);
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
      localStorage.removeItem("position");
      localStorage.removeItem("obstacles");
    }
  });

  const handleClick = () => {
    client.publish("reset", "reset");
  };

  const mapStatus = () => {
    var mapping = {
      0: "Rover is chilling!",
      1: "Rover is driving!",
      2: "Rover is at home!",
    };

    return mapping[drive];
  };

  return (
    <Container>
      <p>MQTT: {connectionStatus}</p>
      {/* <img src={driving} alt="loading..." style={{ height: 50, width: 100 }} /> */}
      <p>{mapStatus()}</p>
      <p>Remaining Range: {range} cm</p>
      <Button variant="outline-light" onClick={handleClick}>
        Reset
      </Button>
    </Container>
  );
};

export default Status;

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
