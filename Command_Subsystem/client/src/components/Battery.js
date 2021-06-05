import React, { useState, useEffect } from "react";
import { useSubscription } from "mqtt-react-hooks";
import BatteryPopup from "./BatteryPopup";
import styled from "styled-components";
import Level0 from "../assets/battery-level-0.svg";
import Level1 from "../assets/battery-level-1.svg";
import Level2 from "../assets/battery-level-2.svg";
import Level3 from "../assets/battery-level-3.svg";
import Level4 from "../assets/battery-level-4.svg";
import ErrorIcon from "../assets/error-triangle.svg";

// component to display current battery levels and health

const Battery = () => {
  const { message } = useSubscription(["battery/status", "rover/status"]);
  const [home, setHome] = useState(true);
  const [battery, setBattery] = useState(100);
  const [show, setShow] = useState(false);
  const [error, setError] = useState(false);
  const [cell0, setcell0] = useState({
    battery_level: 100,
    battery_soh: 100,
    battery_state: 0,
  });
  const [cell1, setcell1] = useState({
    battery_level: 100,
    battery_soh: 100,
    battery_state: 0,
  });
  const [cell2, setcell2] = useState({
    battery_level: 100,
    battery_soh: 100,
    battery_state: 0,
  });

  useEffect(() => {
    if (message) {
      if (message.topic === "battery/status") {
        // set state of each cell
        let data = JSON.parse(message.message);
        let cell = data["cell"];
        if (cell === 0) {
          setcell0(data);
        } else if (cell === 1) {
          setcell1(data);
        } else if (cell === 2) {
          setcell2(data);
        }
      } else if (message.topic === "rover/status") {
        let data = JSON.parse(message.message);
        if (data["drive_status"] === 2) {
          setHome(true);
        } else {
          setHome(false);
        }
      }
    }
  }, [message]);

  useEffect(() => {
    // set state of average
    let avg =
      (cell0["battery_level"] +
        cell1["battery_level"] +
        cell2["battery_level"]) /
      3;

    setBattery(avg);
    setError(false);

    // check if any of the battery states are 5
    if (cell0["battery_state"] === 5) {
      setError(true);
    } else if (cell1["battery_state"] === 5) {
      setError(true);
    } else if (cell2["battery_state"] === 5) {
      setError(true);
    }
  }, [cell0, cell1, cell2]);

  const toggleShow = () => {
    setShow(!show);
  };

  return (
    <React.Fragment>
      <div onClick={toggleShow}>
        {error === true ? (
          <ImgContainer src={ErrorIcon} />
        ) : battery <= 20 ? (
          <ImgContainer src={Level0} />
        ) : battery <= 40 ? (
          <ImgContainer src={Level1} />
        ) : battery <= 60 ? (
          <ImgContainer src={Level2} />
        ) : battery <= 80 ? (
          <ImgContainer src={Level3} />
        ) : (
          <ImgContainer src={Level4} />
        )}
      </div>
      <React.Fragment>
        {show && (
          <BatteryPopup
            toggle={toggleShow}
            cell0={cell0}
            cell1={cell1}
            cell2={cell2}
            ishome={home}
          />
        )}
      </React.Fragment>
    </React.Fragment>
  );
};

export default Battery;

const ImgContainer = styled.img`
  height: 45px;
  width: 45px;
  margin: 4px;
  transition: all 0.6s ease-out;
`;
