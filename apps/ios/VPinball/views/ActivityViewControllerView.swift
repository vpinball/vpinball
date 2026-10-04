import SwiftUI
import UIKit

struct ActivityViewControllerView: UIViewControllerRepresentable {
    @Binding var activityItems: [Any]

    var excludedActivityTypes: [UIActivity.ActivityType]? = nil

    func makeUIViewController(context _: Context) -> UIViewController {
        return UIActivityViewController(activityItems: activityItems,
                                        applicationActivities: nil)
    }

    func updateUIViewController(_: UIViewController, context _: Context) {}
}
