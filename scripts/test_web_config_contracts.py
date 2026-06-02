#!/usr/bin/env python3
"""Contract tests for WebConfig form fields and config/schema fixtures."""

from __future__ import annotations

import json
import re
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
WEB_CONFIG = ROOT / "src" / "WebConfigAsync.h"
CONFIG_H = ROOT / "src" / "Config.h"
PEDALINO_H = ROOT / "src" / "Pedalino.h"
SCHEMA_JSON = ROOT / "data" / "schema.json"


def read_text(relative_path: Path) -> str:
    return relative_path.read_text(encoding="utf-8")


def define_value(source: str, name: str) -> int:
    match = re.search(r"#define\s+{}\s+(\d+)".format(re.escape(name)), source)
    if match is None:
        raise AssertionError("Missing #define {}".format(name))
    return int(match.group(1))


def assert_schema_instance(schema: object, instance: object, path: str = "$") -> None:
    """Validate the schema subset used by data/schema.json fixtures."""
    if not isinstance(schema, dict):
        return

    schema_type = schema.get("type")
    if schema_type == "array":
        if not isinstance(instance, list):
            raise AssertionError("{} should be an array".format(path))
        if "minItems" in schema and len(instance) < schema["minItems"]:
            raise AssertionError("{} has too few items".format(path))
        if "maxItems" in schema and len(instance) > schema["maxItems"]:
            raise AssertionError("{} has too many items".format(path))
        item_schema = schema.get("items")
        if item_schema is not None:
            for index, item in enumerate(instance):
                assert_schema_instance(item_schema, item, "{}[{}]".format(path, index))
    elif schema_type == "object":
        if not isinstance(instance, dict):
            raise AssertionError("{} should be an object".format(path))
        properties = schema.get("properties", {})
        for key, value in instance.items():
            if key in properties:
                assert_schema_instance(properties[key], value, "{}.{}".format(path, key))
    elif schema_type == "string":
        if not isinstance(instance, str):
            raise AssertionError("{} should be a string".format(path))
        if "minLength" in schema and len(instance) < schema["minLength"]:
            raise AssertionError("{} is shorter than minLength".format(path))
        if "maxLength" in schema and len(instance) > schema["maxLength"]:
            raise AssertionError("{} is longer than maxLength".format(path))
    elif schema_type == "integer":
        if not isinstance(instance, int) or isinstance(instance, bool):
            raise AssertionError("{} should be an integer".format(path))
        if "minimum" in schema and instance < schema["minimum"]:
            raise AssertionError("{} is below minimum".format(path))
        if "maximum" in schema and instance > schema["maximum"]:
            raise AssertionError("{} is above maximum".format(path))
    elif schema_type == "number":
        if not isinstance(instance, (int, float)) or isinstance(instance, bool):
            raise AssertionError("{} should be a number".format(path))
    elif schema_type == "boolean":
        if not isinstance(instance, bool):
            raise AssertionError("{} should be a boolean".format(path))

    if "enum" in schema and instance not in schema["enum"]:
        raise AssertionError("{} value {!r} is not in enum".format(path, instance))


class WebConfigContractTest(unittest.TestCase):
    def setUp(self) -> None:
        self.web_config = read_text(WEB_CONFIG)

    def test_incoming_actions_route_and_duplicate_bank_ui_are_registered(self) -> None:
        self.assertIn('httpServer.on("/incoming-actions", HTTP_GET,  http_handle_incoming_actions);', self.web_config)
        self.assertIn('httpServer.on("/incoming-actions", HTTP_POST, http_handle_post_incoming_actions);', self.web_config)
        self.assertIn("Duplicate Bank To", self.web_config)
        self.assertIn("btnGroupCopyIncomingBank", self.web_config)
        self.assertIn('name=\'action\' value=\'copy', self.web_config)
        self.assertIn('command.startsWith("copy")', self.web_config)
        self.assertIn("incoming_actions_copy(b, destination)", self.web_config)

    def test_incoming_actions_rendered_fields_match_post_handler_fields(self) -> None:
        rendered_fields = [
            "name='trig-type-",
            "name='trig-channel-",
            "name='trig-number-",
            "name='trig-valuemode-",
            "name='trig-value-",
            "name='act-type-",
            "name='act-led-",
            "name='act-color-",
            "name='act-slot-",
            "name='act-state-",
            "name='act-bank-",
        ]
        post_fields = [
            'request->arg(String("trig-type-")',
            'request->arg(String("trig-channel-")',
            'request->arg(String("trig-number-")',
            'request->arg(String("trig-valuemode-")',
            'request->arg(String("trig-value-")',
            'request->arg(String("act-type-")',
            'request->arg(String("act-led-")',
            'request->arg(String("act-color-")',
            'request->arg(String("act-slot-")',
            'request->arg(String("act-state-")',
            'request->arg(String("act-bank-")',
        ]

        for field in rendered_fields:
            with self.subTest(field=field):
                self.assertIn(field, self.web_config)

        for field in post_fields:
            with self.subTest(field=field):
                self.assertIn(field, self.web_config)

    def test_sequence_name_field_is_rendered_and_saved_with_same_contract(self) -> None:
        self.assertIn('name=\'sequencename\'', self.web_config)
        self.assertIn('maxlength=\'', self.web_config)
        self.assertIn('request->arg("sequencename")', self.web_config)
        self.assertIn("sequence_label(c)", self.web_config)

    def test_legacy_incoming_actions_warning_is_visible_in_imported_config_flow(self) -> None:
        self.assertIn("Legacy incoming rules detected.", self.web_config)
        self.assertIn("IncomingActions", self.web_config)
        self.assertIn("grouped triggers", self.web_config)


class ConfigFixtureContractTest(unittest.TestCase):
    def setUp(self) -> None:
        self.schema = json.loads(read_text(SCHEMA_JSON))
        self.config_h = read_text(CONFIG_H)
        self.pedalino_h = read_text(PEDALINO_H)

    def schema_node(self, *path: str) -> object:
        node = self.schema
        for key in path:
            node = node[key]
        return node

    def test_sequence_names_schema_matches_runtime_limits_and_config_surfaces(self) -> None:
        sequences = define_value(self.pedalino_h, "SEQUENCES")
        max_sequence_name = define_value(self.pedalino_h, "MAXSEQUENCENAME")

        sequence_names = self.schema_node("properties", "SequenceNames")
        name_schema = self.schema_node("properties", "SequenceNames", "items", "properties", "Name")

        self.assertEqual(sequences, sequence_names["minItems"])
        self.assertEqual(sequences, sequence_names["maxItems"])
        self.assertEqual(max_sequence_name, name_schema["maxLength"])
        self.assertIn('jdoc["SequenceNames"]', self.config_h)
        self.assertIn('String(jp.key().c_str()) == String("SequenceNames")', self.config_h)
        self.assertIn('preferences.putBytes("SeqNames"', self.config_h)
        self.assertIn('preferences.getBytes("SeqNames"', self.config_h)

    def test_incoming_trigger_schema_covers_current_and_legacy_grouped_fixtures(self) -> None:
        incoming = self.schema_node("properties", "IncomingTriggers")
        trigger_props = incoming["items"]["properties"]
        action_props = trigger_props["Actions"]["items"]["properties"]

        current_grouped_trigger = {
            "Bank": 2,
            "TriggerType": "Control Change",
            "Channel": 17,
            "Number": 64,
            "ValueMode": "Exact",
            "Value": 127,
            "Actions": [
                {"Action": "Set Bank", "Bank": 3},
                {"Action": "Set Slot State", "Slot": 4, "State": 1},
                {"Action": "Set Led Color", "Led": 8, "Color": "#ff00aa"},
            ],
        }
        legacy_grouped_trigger = {
            "TriggerType": "Program Change",
            "Channel": 1,
            "Number": 10,
            "ValueMode": "Any",
            "Value": 0,
            "Actions": [{"Action": "Set Bank", "Bank": 2}],
        }

        self.assertIn(current_grouped_trigger["Bank"], trigger_props["Bank"]["enum"])
        self.assertIn(current_grouped_trigger["TriggerType"], trigger_props["TriggerType"]["enum"])
        self.assertIn(current_grouped_trigger["ValueMode"], trigger_props["ValueMode"]["enum"])
        for action in current_grouped_trigger["Actions"]:
            self.assertIn(action["Action"], action_props["Action"]["enum"])

        self.assertNotIn("required", incoming["items"])
        self.assertNotIn("Bank", legacy_grouped_trigger)
        self.assertIn('triggerJson["Bank"] | 0', self.config_h)
        assert_schema_instance(incoming, [current_grouped_trigger, legacy_grouped_trigger])

        invalid_trigger = dict(current_grouped_trigger)
        invalid_trigger["Bank"] = 99
        with self.assertRaises(AssertionError):
            assert_schema_instance(incoming, [invalid_trigger])

    def test_incoming_trigger_persistence_uses_sparse_bank_keys_and_legacy_fallback(self) -> None:
        self.assertIn('snprintf(triggerLabel, sizeof(triggerLabel), "InT%02d", b);', self.config_h)
        self.assertIn('snprintf(countLabel, sizeof(countLabel), "InC%02d", b);', self.config_h)
        self.assertIn('preferences.remove("InTriggers")', self.config_h)
        self.assertIn('preferences.remove("InTrigCnt")', self.config_h)
        self.assertIn('preferences.getUChar("InTrigCnt")', self.config_h)
        self.assertIn('preferences.getBytes("InTriggers"', self.config_h)


if __name__ == "__main__":
    unittest.main()
