import { useState } from "react";

const useMqtt = () => {
  const getMqtt = () => {
    const mqtt = JSON.parse(localStorage.getItem("mqtt"));
    if (mqtt !== null) {
      return mqtt;
    } else {
      return { broker: "", username: "", password: "" };
    }
  };

  const [mqtt, setMqtt] = useState(getMqtt());

  const saveMqtt = (mqtt) => {
    localStorage.setItem("mqtt", JSON.stringify(mqtt));
    setMqtt(mqtt);
  };

  const deleteMqtt = () => {
    localStorage.removeItem("mqtt");
    setMqtt({ broker: "", username: "", password: "" });
  };

  return [mqtt, saveMqtt, deleteMqtt];
};

export default useMqtt;
