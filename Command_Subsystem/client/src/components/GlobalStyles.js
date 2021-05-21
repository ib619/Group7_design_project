import { createGlobalStyle } from "styled-components";

export const GlobalStyles = createGlobalStyle`
  @import url('https://fonts.googleapis.com/css?family=Montserrat:400,600&display=swap');;

  body {
    background: ${({ theme }) => theme.body};
    color: ${({ theme }) => theme.text};
    box-sizing: border-box;
    font-family: 'Montserrat', sans-serif;
    transition: all 0.30s linear;
  }
  `;
