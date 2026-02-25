import Foundation
import SwiftUI

@_silgen_name("VPinball_IOSStartup")
func VPinball_IOSStartup(window: UnsafeMutableRawPointer?) {
    DispatchQueue.main.async {
        if let raw = window,
           let uiWindow = Unmanaged<AnyObject>.fromOpaque(raw).takeUnretainedValue() as? UIWindow,
           let rootViewController = uiWindow.rootViewController
        {
            StatusBarManager.install(on: rootViewController,
                                     hidden: false,
                                     style: .lightContent,
                                     animated: false)

            UISearchBar.appearance().overrideUserInterfaceStyle = .dark

            rootViewController.view.isMultipleTouchEnabled = true

            NotificationCenter.default.removeObserver(rootViewController,
                                                      name: UITextField.textDidChangeNotification,
                                                      object: nil)

            if let sdlTextField = findTextField(in: rootViewController.view) {
                NotificationCenter.default.addObserver(rootViewController,
                                                       selector: NSSelectorFromString("textFieldTextDidChange:"),
                                                       name: UITextField.textDidChangeNotification,
                                                       object: sdlTextField)
            }

            let hostingController = UIHostingController(rootView: VPinballAppView())

            hostingController.view.backgroundColor = UIColor.clear
            hostingController.view.isMultipleTouchEnabled = true

            rootViewController.addChild(hostingController)
            rootViewController.view.addSubview(hostingController.view)
            hostingController.didMove(toParent: rootViewController)

            hostingController.view.translatesAutoresizingMaskIntoConstraints = false
            NSLayoutConstraint.activate([
                hostingController.view.topAnchor.constraint(equalTo: rootViewController.view.topAnchor),
                hostingController.view.bottomAnchor.constraint(equalTo: rootViewController.view.bottomAnchor),
                hostingController.view.leadingAnchor.constraint(equalTo: rootViewController.view.leadingAnchor),
                hostingController.view.trailingAnchor.constraint(equalTo: rootViewController.view.trailingAnchor),
            ])
        }
    }
}

@_silgen_name("VPinball_IOSOpenURL")
func VPinball_IOSOpenURL(url: UnsafePointer<CChar>?) {
    if let ptr = url {
        let raw = String(cString: ptr)
        var fileURL: URL?
        if let url = URL(string: raw), url.scheme == "file" {
            fileURL = url
        } else if raw.hasPrefix("/") {
            let path = raw.removingPercentEncoding ?? raw
            fileURL = URL(fileURLWithPath: path)
        }
        if let fileURL = fileURL {
            DispatchQueue.main.async {
                MainViewModel.shared.openURL = fileURL
            }
        }
    }
}

private func findTextField(in view: UIView) -> UITextField? {
    for subview in view.subviews {
        if let textField = subview as? UITextField {
            return textField
        }
        if let found = findTextField(in: subview) {
            return found
        }
    }
    return nil
}
