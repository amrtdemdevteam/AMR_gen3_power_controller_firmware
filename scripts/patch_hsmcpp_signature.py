Import("env")

from pathlib import Path


def _patch_file(path: Path) -> bool:
    content = path.read_text(encoding="utf-8")
    old = "int HsmEventDispatcherBase::getNextHandlerID()"
    new = "HandlerID_t HsmEventDispatcherBase::getNextHandlerID()"

    if new in content:
        print(f"[hsmcpp-fix] Already patched: {path}")
        return False

    if old not in content:
        print(f"[hsmcpp-fix] Pattern not found: {path}")
        return False

    path.write_text(content.replace(old, new, 1), encoding="utf-8")
    print(f"[hsmcpp-fix] Patched signature in: {path}")
    return True


def apply_hsmcpp_signature_fix() -> None:
    project_dir = Path(env.subst("$PROJECT_DIR"))
    libdeps_dir = project_dir / ".pio" / "libdeps"

    if not libdeps_dir.exists():
        print("[hsmcpp-fix] libdeps directory not found yet, skipping")
        return

    targets = list(libdeps_dir.glob("*/hsmcpp/src/HsmEventDispatcherBase.cpp"))

    if not targets:
        print("[hsmcpp-fix] hsmcpp source not found yet, skipping")
        return

    for file_path in targets:
        _patch_file(file_path)


apply_hsmcpp_signature_fix()
