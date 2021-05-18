import React from "react";
import styled from "styled-components";

const Button = styled.button`
  background: ${({ theme }) => theme.background};
  border: 2px solid ${({ theme }) => theme.toggleBorder};
  color: ${({ theme }) => theme.text};
  width: 4rem;
  border-radius: 30px;
  cursor: pointer;
  font-size:0.8rem;
  margin: 0.1rem;
  padding: 0.6rem;
  }
`;

const Arrow = (props) => {
  return <Button onClick={() => props.onClick()}>{props.name}</Button>;
};

export default Arrow;
