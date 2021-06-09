import React, { useState, useEffect } from "react";
import { useMqttState, useSubscription } from "mqtt-react-hooks";
import { Button } from "react-bootstrap";
import styled from "styled-components";

const RoverStatus = () => {
  const { connectionStatus, client } = useMqttState();
  const { message } = useSubscription([
    "rover/status",
    "rover/status/range",
    "position/update",
  ]);
  const [status, setStatus] = useState({
    drive_status: 2,
    distance_travelled: 0,
    obstacle_detected: 0,
  });
  const [range, setRange] = useState(500); // in cm
  const [pos, setPos] = useState({ x: 0, y: 0, heading: 0 });

  useEffect(() => {
    if (message) {
      let data = JSON.parse(message.message);
      if (message.topic === "rover/status") {
        // on rover status update
        setStatus(data);
      } else if (message.topic === "rover/status/range") {
        // on remaning range update
        setRange(data);
      } else if (message.topic === "position/update") {
        // on position update
        setPos(data);
      }
    }
  }, [message, setStatus, setRange]);

  useEffect(() => {
    // calculate distance, compare with range
    // publish 0,0 t2c
    let dist = Math.sqrt(Math.pow(pos.x, 2) + Math.pow(pos.y, 2)); // in mm
    dist /= 10;
    if (dist >= range - 50) {
      let target = { x: 0, y: 0, speed: 150 };
      client.publish("drive/t2c", JSON.stringify(target));
    }
  }, [pos]);

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
      <p>{mapStatus()}</p>
      <p>Remaining Range: {range} cm</p>
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
