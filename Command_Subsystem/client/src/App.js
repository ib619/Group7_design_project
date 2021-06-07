import React from "react";
import { BrowserRouter as Switch, Route } from "react-router-dom";
import { ThemeProvider } from "styled-components";
import { GlobalStyles } from "./components/GlobalStyles";
import { lightTheme, darkTheme } from "./components/Themes";
import useDarkMode from "./hooks/useDarkMode";
import Login from "./pages/Login";
import RoverStatus from "./components/RoverStatus";
import HomeNavBar from "./components/HomeNavBar";
import MapPage from "./pages/MapPage";
import TargetPage from "./pages/TargetPage";
import DiscretePage from "./pages/DiscretePage";
import { Connector } from "mqtt-react-hooks";
import useToken from "./hooks/useToken";
import useMqtt from "./hooks/useMqtt";
import "../node_modules/bootstrap/dist/css/bootstrap.min.css";

const App = () => {
  const [token, setToken, deleteToken] = useToken();
  const [mqtt, setMqtt, deleteMqtt] = useMqtt({
    broker: "",
    username: "",
    password: "",
  });

  // trying out dark mode
  const [theme, themeToggler, mountedComponent] = useDarkMode();
  const themeMode = theme === "light" ? lightTheme : darkTheme;
  if (!mountedComponent) return <div />;

  if (!token) {
    return <Login setToken={setToken} mqtt={mqtt} setMqtt={setMqtt} />;
  }

  return (
    <ThemeProvider theme={themeMode}>
      <React.Fragment>
        <Connector
          brokerUrl={mqtt["broker"]}
          options={{
            keepalive: 0,
            clientId: mqtt["username"],
            username: mqtt["username"],
            password: mqtt["password"],
          }}
        >
          <GlobalStyles />
          <HomeNavBar
            theme={theme}
            toggleTheme={themeToggler}
            deleteToken={deleteToken}
            deleteMqtt={deleteMqtt}
          />
          <RoverStatus />
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
