import React from "react";
import { useMqttState } from "mqtt-react-hooks";
import { Button } from "react-bootstrap";
import styled from "styled-components";
// import RoverStatus from "./RoverStatus";

const Status = () => {
  /*
   * Status list
   * - Offline
   * - Connected
   * - Reconnecting
   * - Closed
   * - Error: printed in console too
   */

  const { connectionStatus, client } = useMqttState();

  const handleClick = () => {
    client.publish("reset", "reset");
  };

  return (
    <Container>
      <h3>MQTT Status: {connectionStatus}</h3>
      {/* <RoverStatus /> */}
      <Button variant="outline-light" onClick={handleClick}>
        Reset
      </Button>
    </Container>
  );
};

export default Status;

const Container = styled.div`
  display: flex;
  margin: 1rem auto;
  padding: 1rem;
  justify-content: space-between;
`;
