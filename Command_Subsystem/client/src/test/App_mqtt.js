import React, { useState, useEffect } from "react";
import { BrowserRouter as Switch, Route } from "react-router-dom";
import { ThemeProvider } from "styled-components";
import { useDarkMode } from "../components/DarkMode";
import { GlobalStyles } from "../components/GlobalStyles";
import { lightTheme, darkTheme } from "../components/Themes";
import HomeNavBar from "../components/HomeNavBar";
import ArrowPage from "../pages/ArrowPage";
import InputPage from "../pages/InputPage";
import "../node_modules/bootstrap/dist/css/bootstrap.min.css";
import mqtt from "async-mqtt";

var options = {
  // clientId uniquely identifies client
  // choose any string you wish
  clientId: "siting",
  username: "siting",
  password: "password",
};
var client = mqtt.connect("ws://localhost:8080", options);
console.log("created mqtt client object");

client.subscribe("testingg");

const App = () => {
  const [msg, setmsg] = useState("empty");
  // trying out dark mode
  const [theme, themeToggler, mountedComponent] = useDarkMode();
  const themeMode = theme === "light" ? lightTheme : darkTheme;
  if (!mountedComponent) return <div />;

  client.on("connect", function () {
    console.log("connected to mqtt broker");
  });

  client.on("message", function (topic, message) {
    var note = message.toString();
    // Updates React state with message
    setmsg(note);
    console.log(note);
    client.end();
  });

  return (
    <ThemeProvider theme={themeMode}>
      <>
        <GlobalStyles />
        <HomeNavBar theme={theme} toggleTheme={themeToggler} />
        <h1>Message: {msg}</h1>
        {/* /* <Button onClick={() => mqttConnect()}></Button> */}
        <Switch>
          <Route exact path="/" component={ArrowPage}></Route>
          <Route path="/input" render={(props) => <InputPage {...props} />} />
        </Switch>
      </>
    </ThemeProvider>
  );
};

export default App;
