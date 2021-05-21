import React, { useState } from "react";
import Draggable from "react-draggable";
import styled from "styled-components";

const JoyCon = () => {
  const [pos, setPos] = useState({ x: 0, y: 0 });

  const trackPos = (data) => {
    setPos({ x: data.x, y: data.y });
  };

  return (
    <Draggable onDrag={trackPos} position={{ x: 0, y: 0 }}>
      <JoyContainer>
        <div> Joycon </div>
        <div>
          x: {pos.x.toFixed(0)}, y: {pos.y.toFixed(0)}
        </div>
      </JoyContainer>
    </Draggable>
  );
};

export default JoyCon;

const JoyContainer = styled.div`
  display: inline-block;
  background: ${({ theme }) => theme.background};
  color: ${({ theme }) => theme.text};
  max-width: 215px;
  border: 2px solid ${({ theme }) => theme.toggleBorder};
  border-radius: 9px;
  padding: 1em;
`;
