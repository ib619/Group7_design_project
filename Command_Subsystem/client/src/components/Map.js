import React from "react";
import styled from "styled-components";

const Map = (props) => {
  return (
    <>
      <Border>
        <Block coords={props.data}></Block>
      </Border>
      <Coords>
        x: {props.data.x}, y: {props.data.y}
      </Coords>
    </>
  );
};

export default Map;

const Border = styled.div`
  position: relative;
  margin: 1rem auto;
  width: 26rem;
  height: 26rem;
  padding: 0.5rem;
  border: 2px solid black;
`;

const Block = styled.div.attrs((props) => ({
  style: {
    left: props.coords.x,
    top: props.coords.y,
  },
}))`
  background: ${({ theme }) => theme.background};
  color: black;
  padding: 0.5rem;
  width: 1rem;
  height: 1rem;
  position: relative;
`;

const Coords = styled.p`
  background: ${({ theme }) => theme.background};
  color: ${({ theme }) => theme.text};
  text-align: center;
  font-size: 15px;
  padding: 0.5rem;
`;
