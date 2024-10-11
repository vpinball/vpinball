import SwiftUI
import UIKit

struct ActivityViewControllerView: UIViewControllerRepresentable {
    @Binding var activityItems: [Any]

    var excludedActivityTypes: [UIActivity.ActivityType]? = nil

    func makeUIViewController(context _: Context) -> UIViewController {
        let activityViewController = UIActivityViewController(activityItems: activityItems,
                                                              applicationActivities: nil)

        return activityViewController
    }

    func updateUIViewController(_: UIViewController, context _: Context) {}
}
