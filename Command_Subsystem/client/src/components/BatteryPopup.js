import React from "react";
import { Button } from "react-bootstrap";
import { useMqttState } from "mqtt-react-hooks";
import styled from "styled-components";

const BatteryPopup = (props) => {
  const { client } = useMqttState();

  const mapstates = (state) => {
    let map = {
      0: "idle",
      1: "charge",
      2: "charge rest",
      3: "slow discharge",
      4: "discharge rest",
      5: "error",
      6: "const voltage charge",
      7: "recalibration done",
      8: "normal discharge",
      9: "rapid discharge",
      10: "rapid current charge",
    };

    return map[state];
  };

  const handleClick = (e) => {
    client.publish("battery/command", e.target.name);
  };

  return (
    <Popup>
      <div className="box">
        <Content>
          <div className="cell">
            <h3>Cell 0</h3>
            <p>Battery level: {props.cell0["battery_level"].toFixed(2)}%</p>
            <p>Battery soh: {props.cell0["battery_soh"].toFixed(2)}%</p>
            <p>Battery state: {mapstates(props.cell0["battery_state"])}</p>
          </div>
          <div className="cell">
            <h3>Cell 1</h3>
            <p>Battery level: {props.cell1["battery_level"].toFixed(2)}%</p>
            <p>Battery soh: {props.cell1["battery_soh"].toFixed(2)}%</p>
            <p>Battery state: {mapstates(props.cell1["battery_state"])}</p>
          </div>
          <div className="cell">
            <h3>Cell 2</h3>
            <p>Battery level: {props.cell2["battery_level"].toFixed(2)}%</p>
            <p>Battery soh: {props.cell2["battery_soh"].toFixed(2)}%</p>
            <p>Battery state: {mapstates(props.cell2["battery_state"])}</p>
          </div>
        </Content>
        <Buttons>
          <Button
            variant="outline-light"
            name={1}
            onClick={handleClick}
            disabled={!props.ishome}
          >
            Recalibrate
          </Button>
          <Button variant="outline-light" name={2} onClick={handleClick}>
            Reset Cells
          </Button>
        </Buttons>
        <span className="close-icon" onClick={props.toggle}>
          x
        </span>
      </div>
    </Popup>
  );
};

export default BatteryPopup;

const Popup = styled.div`
  position: fixed;
  background: #00000050;
  width: 100%;
  height: 100vh;
  top: 0;
  left: 0;
  z-index: 2;

  .box {
    position: relative;
    width: 70%;
    margin: 0 auto;
    height: auto;
    max-height: 70vh;
    margin-top: calc(100vh - 85vh - 20px);
    background: #393939;
    border-radius: 4px;
    padding: 20px;
    border: 1px solid #999;
    overflow: auto;
    z-index: 2;
  }

  .close-icon {
    content: "x";
    cursor: pointer;
    position: fixed;
    right: calc(15% - 30px);
    top: calc(100vh - 85vh - 33px);
    background: #0f0f0f;
    width: 25px;
    height: 25px;
    border-radius: 50%;
    line-height: 20px;
    text-align: center;
    border: 1px solid #999;
    font-size: 20px;
  }
`;

const Content = styled.div`
  display: flex;
  flex-direction: row;
  justify-content: space-around;

  .cell {
    display: flex;
    flex-direction: column;
    padding: 0.5rem;
    justify-content: center;
  }
`;

const Buttons = styled.div`
  display: flex;
  justify-content: space-evenly;
  padding: 1rem;

  .Button {
    display: flex;
    justify-content: center;
  }
`;
