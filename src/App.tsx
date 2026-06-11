import { useState, useEffect } from "react";
import { invoke } from "@tauri-apps/api/core";
import Sidebar from "./components/Sidebar";
import TitleBar from "./components/TitleBar";
import LoginPage from "./pages/LoginPage";
import HomePage from "./pages/HomePage";
import InstancesPage from "./pages/InstancesPage";
import ModsPage from "./pages/ModsPage";
import SkinsPage from "./pages/SkinsPage";
import SettingsPage from "./pages/SettingsPage";

export interface Account {
  username: string;
  uuid: string;
  access_token: string;
  skin_url: string | null;
}

export default function App() {
  const [page, setPage] = useState("home");
  const [account, setAccount] = useState<Account | null>(null);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    // Проверяем сохранённую сессию при запуске
    invoke<Account | null>("get_saved_session")
      .then((session) => {
        if (session) {
          setAccount(session);
        }
        setLoading(false);
      })
      .catch(() => setLoading(false));
  }, []);

  const handleLogin = (acc: Account) => {
    setAccount(acc);
  };

  const handleLogout = async () => {
    await invoke("logout");
    setAccount(null);
    setPage("home");
  };

  if (loading) {
    return (
      <div className="h-screen bg-wenz-bg flex items-center justify-center">
        <div className="text-wenz-accent text-xl animate-pulse">Загрузка...</div>
      </div>
    );
  }

  // Если не авторизован — показываем логин
  if (!account) {
    return (
      <div className="h-screen bg-wenz-bg flex flex-col overflow-hidden select-none">
        <TitleBar />
        <LoginPage onLogin={handleLogin} />
      </div>
    );
  }

  const renderPage = () => {
    switch (page) {
      case "home":
        return <HomePage account={account} />;
      case "instances":
        return <InstancesPage account={account} />;
      case "mods":
        return <ModsPage />;
      case "skins":
        return <SkinsPage account={account} />;
      case "settings":
        return <SettingsPage />;
      default:
        return <HomePage account={account} />;
    }
  };

  return (
    <div className="h-screen bg-wenz-bg flex flex-col overflow-hidden select-none">
      <TitleBar />
      <div className="flex flex-1 overflow-hidden">
        <Sidebar
          currentPage={page}
          onPageChange={setPage}
          account={account}
          onLogout={handleLogout}
        />
        <main className="flex-1 overflow-y-auto">{renderPage()}</main>
      </div>
    </div>
  );
}
