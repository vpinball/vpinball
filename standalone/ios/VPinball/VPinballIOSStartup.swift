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
