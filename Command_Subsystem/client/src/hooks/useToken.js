import { useState } from "react";

const useToken = () => {
  const getToken = () => {
    const token = localStorage.getItem("token");
    if (token !== null) {
      return token;
    }
  };

  const [token, setToken] = useState(getToken());

  const saveToken = (userToken) => {
    localStorage.setItem("token", "token123");
    setToken(userToken);
  };

  const deleteToken = () => {
    localStorage.removeItem("token");
    setToken();
  };

  return [token, saveToken, deleteToken];
};

export default useToken;
