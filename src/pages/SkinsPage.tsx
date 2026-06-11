import { Upload, RotateCw, Shirt, Palette, Sparkles } from "lucide-react";

const skins = [
  { id: "1", name: "Wenz Classic", type: "Slim", active: true },
  { id: "2", name: "Dark Knight", type: "Classic", active: false },
  { id: "3", name: "Neon Cyber", type: "Slim", active: false },
  { id: "4", name: "Forest Ranger", type: "Classic", active: false },
];

const cosmetics = [
  { id: "1", name: "Драконьи крылья", category: "Крылья", equipped: true, icon: "🐉" },
  { id: "2", name: "Пламенная аура", category: "Аура", equipped: false, icon: "🔥" },
  { id: "3", name: "Алмазная корона", category: "Шапка", equipped: true, icon: "💎" },
  { id: "4", name: "Радужный след", category: "След", equipped: false, icon: "🌈" },
  { id: "5", name: "Щит героя", category: "Спина", equipped: false, icon: "🛡️" },
];

export function SkinsPage() {
  return (
    <div className="animate-fade-in space-y-6">
      <div>
        <h1 className="text-2xl font-bold text-wenz-text">Скины и косметика</h1>
        <p className="mt-1 text-sm text-wenz-muted">
          Настрой внешний вид своего персонажа
        </p>
      </div>

      <div className="grid grid-cols-3 gap-6">
        {/* Skin Preview */}
        <div className="col-span-1 rounded-xl border border-wenz-border bg-wenz-card p-5">
          <h2 className="mb-4 text-sm font-semibold uppercase tracking-wider text-wenz-muted">
            Превью
          </h2>
          <div className="flex aspect-[3/4] items-center justify-center rounded-lg bg-gradient-to-b from-wenz-bg to-wenz-accent/5">
            <div className="text-center">
              <Shirt className="mx-auto h-16 w-16 text-wenz-accent/40" />
              <p className="mt-3 text-xs text-wenz-muted">3D превью скина</p>
            </div>
          </div>
          <div className="mt-4 flex gap-2">
            <button className="flex flex-1 items-center justify-center gap-2 rounded-lg border border-wenz-border py-2 text-xs text-wenz-muted transition-colors hover:border-wenz-accent/30 hover:text-wenz-text">
              <RotateCw className="h-3 w-3" />
              Повернуть
            </button>
          </div>
        </div>

        {/* Skins List */}
        <div className="col-span-2 space-y-6">
          <div className="rounded-xl border border-wenz-border bg-wenz-card p-5">
            <div className="mb-4 flex items-center justify-between">
              <h2 className="text-sm font-semibold uppercase tracking-wider text-wenz-muted">
                Мои скины
              </h2>
              <button className="flex items-center gap-2 rounded-lg bg-wenz-accent/10 px-3 py-1.5 text-xs font-medium text-wenz-accent transition-colors hover:bg-wenz-accent/20">
                <Upload className="h-3 w-3" />
                Загрузить скин
              </button>
            </div>
            <div className="grid grid-cols-2 gap-3">
              {skins.map((skin) => (
                <div
                  key={skin.id}
                  className={`flex items-center gap-3 rounded-lg border p-3 transition-all ${
                    skin.active
                      ? "border-wenz-accent/40 bg-wenz-accent/5"
                      : "border-wenz-border bg-wenz-bg/30 hover:border-wenz-accent/20"
                  }`}
                >
                  <div className="flex h-10 w-10 items-center justify-center rounded-lg bg-wenz-bg">
                    <Palette className={`h-5 w-5 ${skin.active ? "text-wenz-accent" : "text-wenz-muted"}`} />
                  </div>
                  <div className="flex-1">
                    <p className="text-sm font-medium text-wenz-text">{skin.name}</p>
                    <p className="text-xs text-wenz-muted">{skin.type}</p>
                  </div>
                  {skin.active && (
                    <span className="rounded bg-wenz-accent/20 px-2 py-0.5 text-xs font-medium text-wenz-accent">
                      Активный
                    </span>
                  )}
                </div>
              ))}
            </div>
          </div>

          {/* Cosmetics */}
          <div className="rounded-xl border border-wenz-border bg-wenz-card p-5">
            <div className="mb-4 flex items-center justify-between">
              <h2 className="flex items-center gap-2 text-sm font-semibold uppercase tracking-wider text-wenz-muted">
                <Sparkles className="h-4 w-4 text-wenz-purple" />
                Косметика
              </h2>
            </div>
            <div className="grid grid-cols-2 gap-3">
              {cosmetics.map((item) => (
                <div
                  key={item.id}
                  className="flex items-center gap-3 rounded-lg border border-wenz-border bg-wenz-bg/30 p-3 transition-colors hover:border-wenz-accent/20"
                >
                  <span className="text-2xl">{item.icon}</span>
                  <div className="flex-1">
                    <p className="text-sm font-medium text-wenz-text">{item.name}</p>
                    <p className="text-xs text-wenz-muted">{item.category}</p>
                  </div>
                  {item.equipped ? (
                    <span className="rounded bg-wenz-purple/20 px-2 py-0.5 text-xs font-medium text-wenz-purple">
                      Надето
                    </span>
                  ) : (
                    <button className="rounded bg-white/5 px-2 py-0.5 text-xs text-wenz-muted hover:bg-white/10 hover:text-wenz-text">
                      Надеть
                    </button>
                  )}
                </div>
              ))}
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
