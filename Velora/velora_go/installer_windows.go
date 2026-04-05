package main

// #cgo LDFLAGS: -mwindows
import "C"

import (
	"fmt"
	"os"
	"os/exec"
	"path/filepath"

	"golang.org/x/sys/windows/registry"
)

func runInstallerUI() {
	// Dummy installer for now.
	// Normally this would use Walk/lxn or direct win32 API to show a message box.
	fmt.Println("Velora Programming Language v0.1.0")
	fmt.Println("Installing Velora system-wide...")

	exePath, err := os.Executable()
	if err != nil {
		fmt.Println("Error:", err)
		return
	}

	installDir := `C:\Program Files\Velora`
	err = os.MkdirAll(installDir, 0755)
	if err != nil {
		fmt.Println("Error creating install directory. Please run as Administrator.")
		return
	}

	destPath := filepath.Join(installDir, "velora.exe")
	
	// Copy the executable
	input, err := os.ReadFile(exePath)
	if err == nil {
		os.WriteFile(destPath, input, 0755)
	}

	// Make directories for stdlib and examples
	os.MkdirAll(filepath.Join(installDir, "examples"), 0755)
	os.MkdirAll(filepath.Join(installDir, "stdlib"), 0755)

	// Add to PATH
	addToPath(installDir)
	
	// Register extension
	registerExtension(destPath)

	// Create a dummy example
	exampleCode := "// Hello World — Velora\n\nmain:\n    print(\"Hello from Velora!\")\n"
	os.WriteFile(filepath.Join(installDir, "examples", "hello.vel"), []byte(exampleCode), 0644)

	fmt.Println("Velora installed successfully! You can now use the 'velora' command.")

	// Launch tray helper
	exec.Command(destPath, "--tray").Start()
}

func runUninstallUI() {
	fmt.Println("Uninstalling Velora...")
	
	installDir := `C:\Program Files\Velora`
	
	removeFromPath(installDir)
	unregisterExtension()

	fmt.Println("Velora has been uninstalled. You can safely delete C:\\Program Files\\Velora")
}

func addToPath(dir string) {
	key, err := registry.OpenKey(registry.LOCAL_MACHINE, `SYSTEM\CurrentControlSet\Control\Session Manager\Environment`, registry.ALL_ACCESS)
	if err != nil { return }
	defer key.Close()

	pathStr, _, err := key.GetStringValue("Path")
	if err != nil { return }

	if !stringsContains(pathStr, dir) {
		newPath := pathStr + ";" + dir
		key.SetStringValue("Path", newPath)
	}
}

func removeFromPath(dir string) {
	key, err := registry.OpenKey(registry.LOCAL_MACHINE, `SYSTEM\CurrentControlSet\Control\Session Manager\Environment`, registry.ALL_ACCESS)
	if err != nil { return }
	defer key.Close()

	pathStr, _, err := key.GetStringValue("Path")
	if err != nil { return }

	// Very simple removal
	newPath := stringsReplace(pathStr, ";"+dir, "")
	newPath = stringsReplace(newPath, dir+";", "")
	key.SetStringValue("Path", newPath)
}

func stringsContains(s, substr string) bool {
	return len(s) >= len(substr) && s[len(s)-len(substr):] == substr || 
	       len(s) >= len(substr) && s[:len(substr)] == substr
}

func stringsReplace(s, old, new string) string {
	for i := 0; i <= len(s)-len(old); i++ {
		if s[i:i+len(old)] == old {
			return s[:i] + new + s[i+len(old):]
		}
	}
	return s
}

func registerExtension(exePath string) {
	key, _, err := registry.CreateKey(registry.CLASSES_ROOT, `.vel`, registry.ALL_ACCESS)
	if err == nil {
		key.SetStringValue("", "VeloraSourceFile")
		key.Close()
	}

	key, _, err = registry.CreateKey(registry.CLASSES_ROOT, `VeloraSourceFile\shell\open\command`, registry.ALL_ACCESS)
	if err == nil {
		key.SetStringValue("", `"`+exePath+`" run "%1"`)
		key.Close()
	}
}

func unregisterExtension() {
	registry.DeleteKey(registry.CLASSES_ROOT, `.vel`)
	registry.DeleteKey(registry.CLASSES_ROOT, `VeloraSourceFile\shell\open\command`)
	registry.DeleteKey(registry.CLASSES_ROOT, `VeloraSourceFile\shell\open`)
	registry.DeleteKey(registry.CLASSES_ROOT, `VeloraSourceFile\shell`)
	registry.DeleteKey(registry.CLASSES_ROOT, `VeloraSourceFile`)
}

func attachConsole() {
	// Not implemented in dummy version
}
