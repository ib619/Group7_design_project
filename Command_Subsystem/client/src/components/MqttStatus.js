import React, { useEffect, useState } from "react";
import { useMqttState, useSubscription } from "mqtt-react-hooks";

const Status = () => {
  /*
   * Status list
   * - Offline
   * - Connected
   * - Reconnecting
   * - Closed
   * - Error: printed in console too
   */

  const [msg, setmsg] = useState("");
  const { connectionStatus } = useMqttState();
  const { message } = useSubscription("testingg");

  useEffect(() => {
    if (message) setmsg(message.message);
  }, [message]);

  return (
    <>
      <h1>{`Status: ${connectionStatus}`}</h1>
      <span>{`Message: ${msg}`}</span>
    </>
  );
};

export default Status;
