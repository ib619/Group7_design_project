import React from "react";
import { useMqttState } from "mqtt-react-hooks";

const Status = () => {
  /*
   * Status list
   * - Offline
   * - Connected
   * - Reconnecting
   * - Closed
   * - Error: printed in console too
   */

  const { connectionStatus } = useMqttState();

  // TODO: make pretty
  return (
    <>
      <h3>MQTT Status: {connectionStatus}</h3>
    </>
  );
};

export default Status;
