import React from "react";
import { BrowserRouter as Switch, Route } from "react-router-dom";
import { ThemeProvider } from "styled-components";
import { useDarkMode } from "./components/DarkMode";
import { GlobalStyles } from "./components/GlobalStyles";
import { lightTheme, darkTheme } from "./components/Themes";
import HomeNavBar from "./components/HomeNavBar";
import ArrowPage from "./pages/ArrowPage";
import InputPage from "./pages/InputPage";
import "../node_modules/bootstrap/dist/css/bootstrap.min.css";

const App = () => {
  // trying out dark mode
  const [theme, themeToggler, mountedComponent] = useDarkMode();
  const themeMode = theme === "light" ? lightTheme : darkTheme;
  if (!mountedComponent) return <div />;

  return (
    <ThemeProvider theme={themeMode}>
      <>
        <GlobalStyles />
        <HomeNavBar theme={theme} toggleTheme={themeToggler} />
        <Switch>
          <Route exact path="/" component={ArrowPage}></Route>
          <Route exact path="/input" component={InputPage}></Route>
        </Switch>
      </>
    </ThemeProvider>
  );
};

export default App;
