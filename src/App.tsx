import { Routes, Route } from "react-router-dom";
import { Sidebar } from "./components/Sidebar";
import { TitleBar } from "./components/TitleBar";
import { HomePage } from "./pages/HomePage";
import { InstancesPage } from "./pages/InstancesPage";
import { ModsPage } from "./pages/ModsPage";
import { SettingsPage } from "./pages/SettingsPage";
import { SkinsPage } from "./pages/SkinsPage";

export default function App() {
  return (
    <div className="flex h-screen w-screen flex-col overflow-hidden bg-wenz-bg">
      <TitleBar />
      <div className="flex flex-1 overflow-hidden">
        <Sidebar />
        <main className="flex-1 overflow-y-auto p-6">
          <Routes>
            <Route path="/" element={<HomePage />} />
            <Route path="/instances" element={<InstancesPage />} />
            <Route path="/mods" element={<ModsPage />} />
            <Route path="/skins" element={<SkinsPage />} />
            <Route path="/settings" element={<SettingsPage />} />
          </Routes>
        </main>
      </div>
    </div>
  );
}
