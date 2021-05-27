import React, { useState, useEffect } from "react";
import { useSubscription } from "mqtt-react-hooks";

const RoverStatus = () => {
  const { message } = useSubscription("rover/status");
  const [status, setStatus] = useState({});

  useEffect(() => {
    if (message && message.topic === "rover/status") {
      setStatus(JSON.parse(message.message));
    }
  }, [message]);

  return (
    <>
      <h3>Rover Status: {status["drive_status"]} </h3>
    </>
  );
};

export default RoverStatus;
