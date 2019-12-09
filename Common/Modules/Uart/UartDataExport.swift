//
//  UartDataExport.swift
//  Bluefruit Connect
//
//  Created by Antonio García on 10/01/16.
//  Copyright © 2016 Adafruit. All rights reserved.
//

import Foundation

class UartDataExport {

    // MARK: - Export formatters
    static func packetsAsText(_ packets: [UartPacket], isHexFormat: Bool) -> String? {
        // Compile all data
        var data = Data()
        for packet in packets {
            data.append(packet.data)
        }

        // Convert to text
        var text: String?
        if isHexFormat {
            text = hexDescription(data: data)
        } else {
            text = String(data:data, encoding: .utf8)
        }
        if (text == "hello") {
            sendNotification(text: text)
        }

        return text
    }
    
    static func sendNotification(text: String?) {

        // letting center be the current Notification Center - no changes needed
        let center = UNUserNotificationCenter.current()
            
        
        // letting content be the Notification Content of the Notification being sent - changes needed
        let content = UNMutableNotificationContent()
        content.title = "Notification title" // title of the notification - on messages notification: the sender
        content.body = "Notification body" // body of the notification - on messages notification: the preview of the text
            
            
        // setting up the trigger for the notification
            
        // currently set to send a notification 1 second after getting data
        let date = Date().addingTimeInterval(1)
        let dateComponents = Calendar.current.dateComponents([.year, .month, .day, .hour, .minute, .second], from: date)
        // declaring the trigger - changes needed
        let trigger = UNCalendarNotificationTrigger(dateMatching: dateComponents, repeats: false)
        
        // letting the uuidString = UUID generated string - no changes needed
        let uuidString = UUID().uuidString
            
        // creating request for notification - no changes needed if constants' names don't change
        let request = UNNotificationRequest(identifier: uuidString, content: content, trigger: trigger)
            
        // adding request to the Notification Center - no changes needed if constants' names don't change
        center.add(request) { (error) in
        }
    }

    static func packetsAsCsv(_ packets: [UartPacket], isHexFormat: Bool) -> String? {
        var text = "Timestamp,Mode,Data\r\n"        // csv Header

        let timestampDateFormatter = DateFormatter()
        timestampDateFormatter.setLocalizedDateFormatFromTemplate("HH:mm:ss:SSS")

        for packet in packets {
            let date = Date(timeIntervalSinceReferenceDate: packet.timestamp)

            let dateString = timestampDateFormatter.string(from: date).replacingOccurrences(of: ",", with: ".")     //  comma messes with csv, so replace it by a point
            let mode = packet.mode == .rx ? "RX" : "TX"
            var dataString: String?
            if isHexFormat {
                dataString = hexDescription(data: packet.data)
            } else {
                dataString = String(data: packet.data, encoding: .utf8)
            }

            // Remove newline characters from data (it messes with the csv format and Excel wont recognize it)
            dataString = dataString?.trimmingCharacters(in: CharacterSet.newlines) ?? ""
            text += "\(dateString),\(mode),\"\(dataString!)\"\r\n"
        }

        return text
    }

    static func packetsAsJson(_ packets: [UartPacket], isHexFormat: Bool) -> String? {

        var jsonItemsArray: [Any] = []

        for packet in packets {
            let date = Date(timeIntervalSinceReferenceDate: packet.timestamp)
            let unixDate = date.timeIntervalSince1970
            let mode = packet.mode == .rx ? "RX" : "TX"

            var dataString: String?
            if isHexFormat {
                dataString = hexDescription(data: packet.data)
            } else {
                dataString = String(data: packet.data, encoding: .utf8)
            }

            if let dataString = dataString {
                let jsonItemDictionary: [String: Any] = [
                    "timestamp": unixDate,
                    "mode": mode,
                    "data": dataString
                ]
                jsonItemsArray.append(jsonItemDictionary)
            }
        }

        let jsonRootDictionary: [String: Any] = [
            "items": jsonItemsArray
        ]

        // Create Json Data
        var data: Data?
        do {
            data = try JSONSerialization.data(withJSONObject: jsonRootDictionary, options: .prettyPrinted)
        } catch {
            DLog("Error serializing json data")
        }

        // Create Json String
        var result: String?
        if let data = data {
            result = String(data: data, encoding: .utf8)
        }

        return result
    }

    static func packetsAsXml(_ packets: [UartPacket], isHexFormat: Bool) -> String? {

        #if os(OSX)
        let xmlRootElement = XMLElement(name: "uart")

        for packet in packets {
            let date = Date(timeIntervalSinceReferenceDate: packet.timestamp)
            let unixDate = date.timeIntervalSince1970
            let mode = packet.mode == .rx ? "RX" : "TX"

            var dataString: String?
            if isHexFormat {
                dataString = hexDescription(data: packet.data)
            } else {
                dataString = String(data: packet.data, encoding: .utf8)
            }

            if let dataString = dataString {

                let xmlItemElement = XMLElement(name: "item")
                xmlItemElement.addChild(XMLElement(name: "timestamp", stringValue:"\(unixDate)"))
                xmlItemElement.addChild(XMLElement(name: "mode", stringValue:mode))
                let dataNode = XMLElement(kind: .text, options: XMLNode.Options.nodeIsCDATA)
                dataNode.name = "data"
                dataNode.stringValue = dataString
                xmlItemElement.addChild(dataNode)

                xmlRootElement.addChild(xmlItemElement)
            }
        }

        let xml = XMLDocument(rootElement: xmlRootElement)
        let result = xml.xmlString(withOptions: Int(XMLNode.Options.nodePrettyPrint.rawValue))

        return result

        #else
            // TODO: implement for iOS

            return nil

        #endif
    }

    static func packetsAsBinary(_ packets: [UartPacket]) -> Data? {
        guard !packets.isEmpty else { return nil }

        var result = Data()
        for packet in packets {
            result.append(packet.data)
        }

        return result
    }
}
