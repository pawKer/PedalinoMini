#!/usr/bin/env python3
"""Tests for config surface validation helpers."""

from __future__ import annotations

import unittest
from contextlib import redirect_stderr
from io import StringIO

from scripts import validate_config_surfaces as validator


class ValidateConfigSurfacesTest(unittest.TestCase):
    def test_action_mapping_helpers_parse_and_round_trip_config_snippet(self) -> None:
        config_h = """
byte ActionStringToEnum(String msg)
{
  if      (msg.equals("None"))                return PED_EMPTY;
  else if (msg.equals("Set Led Color"))       return PED_ACTION_LED_COLOR;
  else if (msg.equals("Set Slot State"))      return PED_ACTION_SET_SLOT_STATE;
  else if (msg.equals("WLED"))                return PED_ACTION_WLED;
}

String ActionEnumToString(byte msg)
{
  switch (msg) {
    case PED_EMPTY:
      return "None";
      break;
    case PED_ACTION_LED_COLOR:
      return "Set Led Color";
      break;
    case PED_ACTION_SET_SLOT_STATE:
      return "Set Slot State";
      break;
    case PED_ACTION_WLED:
      return "WLED";
      break;
  }
}
"""

        label_to_symbol = validator.action_string_to_enum_map(config_h)
        symbol_to_label = validator.action_enum_to_string_map(config_h)

        self.assertEqual(
            {
                "None": "PED_EMPTY",
                "Set Led Color": "PED_ACTION_LED_COLOR",
                "Set Slot State": "PED_ACTION_SET_SLOT_STATE",
                "WLED": "PED_ACTION_WLED",
            },
            label_to_symbol,
        )
        self.assertEqual("Set Led Color", symbol_to_label["PED_ACTION_LED_COLOR"])
        self.assertEqual("WLED", symbol_to_label["PED_ACTION_WLED"])
        validator.assert_config_round_trips(label_to_symbol, symbol_to_label)

    def test_config_round_trip_rejects_mismatched_labels(self) -> None:
        with redirect_stderr(StringIO()):
            with self.assertRaises(SystemExit):
                validator.assert_config_round_trips(
                    {"Set Bank": "PED_ACTION_BANK_PLUS"},
                    {"PED_ACTION_BANK_PLUS": "Bank+"},
                )

    def test_schema_enum_reads_nested_enum_list(self) -> None:
        schema = {
            "properties": {
                "IncomingTriggers": {
                    "items": {
                        "properties": {
                            "Actions": {
                                "items": {
                                    "properties": {
                                        "Action": {"enum": ["Set Bank", "Set Slot State"]}
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        labels = validator.schema_enum(
            schema,
            "properties",
            "IncomingTriggers",
            "items",
            "properties",
            "Actions",
            "items",
            "properties",
            "Action",
            "enum",
        )

        self.assertEqual(["Set Bank", "Set Slot State"], labels)

    def test_unique_check_rejects_duplicate_values(self) -> None:
        with redirect_stderr(StringIO()):
            with self.assertRaises(SystemExit):
                validator.assert_unique("example", ["Sequence", "Set Bank", "Sequence"])

    def test_required_label_check_rejects_missing_schema_values(self) -> None:
        with redirect_stderr(StringIO()):
            with self.assertRaises(SystemExit):
                validator.assert_contains_required_labels(
                    "IncomingTriggers.Actions.Action schema enum",
                    ["Set Led Color", "Set Bank"],
                    ["Set Led Color", "Set Slot State", "Set Bank"],
                )

    def test_web_option_labels_parse_actual_options_for_selected_expression(self) -> None:
        web_config = """
page += F(">Set Led Color</option>");
page += F(">Set Slot State</option>");
page += F(">WLED</option>");
page += F("     case 'Set Bank':");
page += F("Set Bank");
page += F("<option value='");
page += PED_ACTION_LED_COLOR;
page += F("'");
if (act->midiMessage == PED_ACTION_LED_COLOR) page += F(" selected");
page += F(">Set Led Color</option>");
page += F("<option value='");
page += PED_ACTION_WLED;
page += F("'");
if (act->midiMessage == PED_ACTION_WLED) page += F(" selected");
page += F(">WLED</option>");
page += F("<option value='");
page += PED_ACTION_BANK;
page += F("'");
if (sequences[s-1][i-1].midiMessage == PED_ACTION_BANK) page += F(" selected");
page += F(">Set Bank</option>");
"""

        self.assertEqual(
            {"Set Led Color", "WLED"},
            validator.web_option_labels_for_selected_expression(
                web_config,
                "act->midiMessage ==",
            ),
        )

    def test_web_option_label_check_rejects_missing_required_option_in_group(self) -> None:
        web_config = """
page += F(">Set Led Color</option>");
page += F(">WLED</option>");
page += F("     case 'Set Slot State':");
page += F("<option value='");
page += PED_ACTION_LED_COLOR;
page += F("'");
if (bankTriggers[i].actions[a].targetAction == PED_ACTION_LED_COLOR) page += F(" selected");
page += F(">Set Led Color</option>");
page += F("<option value='");
page += PED_ACTION_SET_SLOT_STATE;
page += F("'");
if (bankTriggers[i].actions[a].targetAction == PED_ACTION_SET_SLOT_STATE) page += F(" selected");
page += F(">Set Slot State</option>");
page += F("<option value='");
page += PED_ACTION_WLED;
page += F("'");
if (bankTriggers[i].actions[a].targetAction == PED_ACTION_WLED) page += F(" selected");
page += F(">WLED</option>");
page += F("<option value='");
page += PED_ACTION_BANK;
page += F("'");
if (sequences[s-1][i-1].midiMessage == PED_ACTION_BANK) page += F(" selected");
page += F(">Set Bank</option>");
"""

        with redirect_stderr(StringIO()):
            with self.assertRaises(SystemExit):
                validator.assert_web_option_group_includes_required_labels(
                    "Incoming target-action Web UI options",
                    web_config,
                    "bankTriggers[i].actions[a].targetAction ==",
                    ["Set Led Color", "Set Slot State", "Set Bank", "WLED"],
                )


if __name__ == "__main__":
    unittest.main()
