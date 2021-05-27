import React from "react";
import Arrow from "./Arrow";
import styled from "styled-components";
import ArrowKeysReact from "arrow-keys-react";

const Controller = () => {
  // max: 384
  var maxVal = 384;
  ArrowKeysReact.config({
    left: () => {
      leftFunc();
    },
    right: () => {
      rightFunc();
    },
    up: () => {
      upFunc();
    },
    down: () => {
      downFunc();
    },
  });

  const upFunc = () => {
    if (props.data.y > 0) {
      props.data.y -= 4;
    }
    props.setPos({ ...props.data, y: props.data.y });
  };

  const downFunc = () => {
    if (props.data.y < maxVal) {
      props.data.y += 4;
    }
    props.setPos({ ...props.data, y: props.data.y });
  };

  const leftFunc = () => {
    if (props.data.x > 0) {
      props.data.x -= 4;
    }
    props.setPos({ ...props.data, x: props.data.x });
  };

  const rightFunc = () => {
    if (props.data.x < maxVal) {
      props.data.x += 4;
    }
    props.setPos({ ...props.data, x: props.data.x });
  };

  // TODO: long hold effect
  // TODO: allow keyboard press to correspond to the arrow effects

  return (
    <div {...ArrowKeysReact.events} tabIndex="0">
      <Keypad>
        <div className="center">
          <Arrow name="Up" onClick={() => upFunc()}></Arrow>
        </div>
        <div>
          <Arrow name="Left" onClick={() => leftFunc()}></Arrow>
          <Arrow name="Down" onClick={() => downFunc()}></Arrow>
          <Arrow name="Right" onClick={() => rightFunc()}></Arrow>
        </div>
      </Keypad>
    </div>
  );
};

export default Controller;

const Keypad = styled.div`
  margin: 1.5rem auto;
  color: black;
  width: 17em;
  border: 3px solid black;
  border-radius: 7em;
  padding: 1.5em;
  display: grid;
  justify-content: center;
  align-items: center;

  .center {
    position: relative;
    left: 33%;
  }
`;
