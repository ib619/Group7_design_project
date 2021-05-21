import React from "react";
import { func, string } from "prop-types";
import styled from "styled-components";
import Icon from "./Icon";

const Button = styled.button`
  background: ${({ theme }) => theme.background};
  border: 2px solid ${({ theme }) => theme.toggleBorder};
  color: ${({ theme }) => theme.text};
  border-radius: 30px;
  cursor: pointer;
  font-size:0.8rem;
  padding: 0.6rem;
  margin: 0.5rem;
  }
`;

const Toggle = ({ theme, toggleTheme }) => {
  return (
    <Button onClick={toggleTheme}>
      <Icon name={theme === "dark" ? "light" : "dark"} />
    </Button>
  );
};

Toggle.propTypes = {
  theme: string.isRequired,
  toggleTheme: func.isRequired,
};

export default Toggle;
