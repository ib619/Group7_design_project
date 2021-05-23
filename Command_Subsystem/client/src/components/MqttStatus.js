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
      <h1>{`Status: ${connectionStatus}`}</h1>
    </>
  );
};

export default Status;
