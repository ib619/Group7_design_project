import React, { useState, useEffect } from "react";
import { BrowserRouter as Switch, Route } from "react-router-dom";
import { ThemeProvider } from "styled-components";
import { useDarkMode } from "./components/DarkMode";
import { GlobalStyles } from "./components/GlobalStyles";
import { lightTheme, darkTheme } from "./components/Themes";
import Status from "./components/MqttStatus";
import HomeNavBar from "./components/HomeNavBar";
import ArrowPage from "./pages/ArrowPage";
import TargetPage from "./pages/TargetPage";
import DiscretePage from "./pages/DiscretePage";
import { Connector, useSubscription } from "mqtt-react-hooks";
import "../node_modules/bootstrap/dist/css/bootstrap.min.css";

const App = () => {
  // for the map stuff
  const [pos, setPos] = useState({ x: 0, y: 0 });
  const { message } = useSubscription("position/update");

  // // on refresh
  // useEffect(() => {
  //   const data = JSON.parse(localStorage.getItem("position"));
  //   if (data) setPos(data);
  // }, []);

  // on position update
  useEffect(() => {
    if (message && message.topic === "position/update") {
      setPos(JSON.parse(message.message));
      localStorage.setItem("position", JSON.stringify(pos));
    }
  }, [message, pos]);

  // trying out dark mode
  const [theme, themeToggler, mountedComponent] = useDarkMode();
  const themeMode = theme === "light" ? lightTheme : darkTheme;
  if (!mountedComponent) return <div />;

  return (
    <ThemeProvider theme={themeMode}>
      <>
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
            <Route
              exact
              path="/"
              render={(props) => (
                <ArrowPage {...props} pos={pos} setPos={setPos} />
              )}
            />
            <Route
              path="/target"
              render={(props) => <TargetPage {...props} pos={pos} />}
            />
            <Route
              path="/discrete"
              render={(props) => <DiscretePage {...props} pos={pos} />}
            />
          </Switch>
        </Connector>
      </>
    </ThemeProvider>
  );
};

export default App;
