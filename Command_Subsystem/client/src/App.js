import React from "react";
import { BrowserRouter as Switch, Route } from "react-router-dom";
import { ThemeProvider } from "styled-components";
import { useDarkMode } from "./components/DarkMode";
import { GlobalStyles } from "./components/GlobalStyles";
import { lightTheme, darkTheme } from "./components/Themes";
import Status from "./components/MqttStatus";
import HomeNavBar from "./components/HomeNavBar";
import MapPage from "./pages/MapPage";
import TargetPage from "./pages/TargetPage";
import DiscretePage from "./pages/DiscretePage";
import { Connector } from "mqtt-react-hooks";
import "../node_modules/bootstrap/dist/css/bootstrap.min.css";

const App = () => {
  // trying out dark mode
  const [theme, themeToggler, mountedComponent] = useDarkMode();
  const themeMode = theme === "light" ? lightTheme : darkTheme;
  if (!mountedComponent) return <div />;

  return (
    <ThemeProvider theme={themeMode}>
      <React.Fragment>
        <Connector
          brokerUrl="ws://localhost:8080"
          options={{
            keepalive: 0,
            clientId: "react",
            username: "siting",
            password: "password",
          }}
        >
          <GlobalStyles />
          <HomeNavBar theme={theme} toggleTheme={themeToggler} />
          <Status />
          <Switch>
            <Route exact path="/" component={MapPage} />
            <Route exact path="/target" component={TargetPage} />
            <Route exact path="/discrete" component={DiscretePage} />
          </Switch>
        </Connector>
      </React.Fragment>
    </ThemeProvider>
  );
};

export default App;
