import ObjectiveC.runtime
import SwiftUI
import UIKit

class StatusBarManager {
    static let shared = StatusBarManager()

    private weak var attachedRootViewController: UIViewController?
    private weak var owningViewController: UIViewController?
    private var hidden = false
    private var style: UIStatusBarStyle = .default
    private var animated = true

    private static var kInstalledKey: UInt8 = 0
    private static var kHiddenKey: UInt8 = 0
    private static var kStyleKey: UInt8 = 0

    func attach(to root: UIViewController) {
        attachedRootViewController = root
        let top = topMost(from: root)
        owningViewController = top
        swizzleIfNeeded(on: top)
    }

    func setHidden(_ hidden: Bool, style: UIStatusBarStyle? = nil, animated: Bool = true) {
        guard attachedRootViewController != nil else { return }
        self.hidden = hidden
        if let s = style { self.style = s }
        self.animated = animated
        ensureAttached()
        guard let viewController = owningViewController else { return }
        objc_setAssociatedObject(viewController,
                                 &Self.kHiddenKey,
                                 NSNumber(value: self.hidden),
                                 .OBJC_ASSOCIATION_RETAIN_NONATOMIC)
        objc_setAssociatedObject(viewController,
                                 &Self.kStyleKey,
                                 NSNumber(value: self.style.rawValue),
                                 .OBJC_ASSOCIATION_RETAIN_NONATOMIC)
        if animated {
            UIView.animate(withDuration: 0.25) { viewController.setNeedsStatusBarAppearanceUpdate() }
        } else {
            viewController.setNeedsStatusBarAppearanceUpdate()
        }
    }

    func reattachToTopMost() {
        guard let rootViewController = attachedRootViewController else { return }
        let topMostViewController = topMost(from: rootViewController)
        if topMostViewController !== owningViewController {
            owningViewController = topMostViewController
            swizzleIfNeeded(on: topMostViewController)
            setHidden(hidden,
                      style: style,
                      animated: false)
        }
    }

    func ensureAttached() {
        guard let rootViewController = attachedRootViewController else { return }
        if owningViewController == nil { owningViewController = topMost(from: rootViewController) }
        if let viewController = owningViewController, objc_getAssociatedObject(viewController, &Self.kInstalledKey) == nil {
            swizzleIfNeeded(on: viewController)
        }
    }

    func swizzleIfNeeded(on viewController: UIViewController) {
        if objc_getAssociatedObject(viewController, &Self.kInstalledKey) != nil { return }
        guard let origClass: AnyClass = object_getClass(viewController) else { return }
        let subclassName = "\(NSStringFromClass(origClass))_StatusBarProxy"
        let subclass: AnyClass
        if let existing = NSClassFromString(subclassName) {
            subclass = existing
        } else {
            guard let sub = objc_allocateClassPair(origClass, subclassName, 0) else { return }

            let prefersHiddenBlock: @convention(block) (AnyObject) -> Bool = { obj in
                (objc_getAssociatedObject(obj, &StatusBarManager.kHiddenKey) as? NSNumber)?.boolValue ?? false
            }
            class_addMethod(sub,
                            #selector(getter: UIViewController.prefersStatusBarHidden),
                            imp_implementationWithBlock(prefersHiddenBlock),
                            "B@:")

            let styleBlock: @convention(block) (AnyObject) -> Int = { obj in
                (objc_getAssociatedObject(obj, &StatusBarManager.kStyleKey) as? NSNumber)?.intValue
                    ?? UIStatusBarStyle.default.rawValue
            }
            class_addMethod(sub,
                            #selector(getter: UIViewController.preferredStatusBarStyle),
                            imp_implementationWithBlock(styleBlock),
                            "q@:")

            let childNilBlock: @convention(block) (AnyObject) -> AnyObject? = { _ in nil }
            class_addMethod(sub,
                            #selector(getter: UIViewController.childForStatusBarHidden),
                            imp_implementationWithBlock(childNilBlock),
                            "@@:")
            class_addMethod(sub,
                            #selector(getter: UIViewController.childForStatusBarStyle),
                            imp_implementationWithBlock(childNilBlock),
                            "@@:")

            objc_registerClassPair(sub)
            subclass = sub
        }

        object_setClass(viewController, subclass)
        objc_setAssociatedObject(viewController,
                                 &Self.kInstalledKey,
                                 true,
                                 .OBJC_ASSOCIATION_RETAIN_NONATOMIC)
        objc_setAssociatedObject(viewController,
                                 &Self.kHiddenKey,
                                 NSNumber(value: hidden),
                                 .OBJC_ASSOCIATION_RETAIN_NONATOMIC)
        objc_setAssociatedObject(viewController,
                                 &Self.kStyleKey,
                                 NSNumber(value: style.rawValue),
                                 .OBJC_ASSOCIATION_RETAIN_NONATOMIC)
        viewController.setNeedsStatusBarAppearanceUpdate()
    }

    func topMost(from root: UIViewController) -> UIViewController {
        var viewController = root
        while let presentedViewController = viewController.presentedViewController {
            viewController = presentedViewController
        }
        return viewController
    }
}

extension StatusBarManager {
    static func install(on root: UIViewController,
                        hidden: Bool = false,
                        style: UIStatusBarStyle = .default,
                        animated: Bool = false)
    {
        Task { @MainActor in
            StatusBarManager.shared.attach(to: root)
            StatusBarManager.shared.setHidden(hidden,
                                              style: style,
                                              animated: animated)
        }
    }
}
