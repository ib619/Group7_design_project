import React, { useState, useEffect } from "react";
import { useMqttState, useSubscription } from "mqtt-react-hooks";
import { Button } from "react-bootstrap";
import FormAlert from "./FormAlert";
import styled from "styled-components";

const RoverStatus = () => {
  const { connectionStatus, client } = useMqttState();
  const { message } = useSubscription([
    "rover/status",
    "rover/status/energy",
    "position/update",
  ]);
  const [status, setStatus] = useState({
    drive_status: 2,
    distance_travelled: 0,
    obstacle_detected: 0,
  });
  const [range, setRange] = useState(500); // in cm
  const [pos, setPos] = useState({ x: 0, y: 0, heading: 0 });
  const [show, setShow] = useState(false);

  useEffect(() => {
    if (message) {
      let data = JSON.parse(message.message);
      if (message.topic === "rover/status") {
        // on rover status update
        setStatus(data);
      } else if (message.topic === "rover/status/energy") {
        // on remaning range update
        setRange(data["range"]);
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
    dist /= 10; // in cm
    if (dist >= range - 30) {
      let target = { x: 0, y: 0, speed: 150 };
      client.publish("drive/t2c", JSON.stringify(target));
      setShow(true);
    }
  }, [pos, range]);

  useEffect(() => {
    if (status["drive_status"] === 2) {
      setShow(false);
    }
  }, [status]);

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
      <React.Fragment>
        <FormAlert home={true} show={show} />
      </React.Fragment>
      <div className="status">
        <p>MQTT: {connectionStatus}</p>
        <p>{mapStatus()}</p>
        <p>Total distance travelled: {status["distance_travelled"]} mm</p>
        <Button name={1} variant="outline-light" onClick={handleClick}>
          Reset
        </Button>
        <Button name={2} variant="outline-danger" onClick={handleClick}>
          Stop
        </Button>
      </div>
    </Container>
  );
};

export default RoverStatus;

const Container = styled.div`
  display: flex;
  flex-direction: column;

  .status {
    display: flex;
    flex-direction: row;
    justify-content: space-between;
    margin: 1rem;
    padding: 0.5rem;
  }

  .p {
    color: ${({ theme }) => theme.text};
    text-align: center;
    transition: all 0.5s ease-out;
  }
`;
