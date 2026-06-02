#!/usr/bin/env python3
"""Validate PedalinoMini config/schema/action surface consistency."""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path
from typing import Dict, List, Set


ROOT = Path(__file__).resolve().parents[1]
CONFIG_H = ROOT / "src" / "Config.h"
WEB_CONFIG = ROOT / "src" / "WebConfigAsync.h"
SCHEMA_JSON = ROOT / "data" / "schema.json"


def fail(message: str) -> None:
    print("ERROR: {}".format(message), file=sys.stderr)
    raise SystemExit(1)


def read_text(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        return path.read_text(encoding="latin-1")


def action_string_to_enum_map(config_h: str) -> Dict[str, str]:
    pairs = re.findall(
        r'msg\.equals\("([^"]+)"\)\)\s+return\s+([A-Z0-9_]+);',
        config_h,
    )
    if not pairs:
        fail("No ActionStringToEnum() entries found in src/Config.h")
    return dict(pairs)


def action_enum_to_string_map(config_h: str) -> Dict[str, str]:
    pairs = re.findall(
        r'case\s+([A-Z0-9_]+):\s*[\r\n\s]*return\s+"([^"]+)";',
        config_h,
    )
    if not pairs:
        fail("No ActionEnumToString() entries found in src/Config.h")
    return dict(pairs)


def schema_enum(schema: Dict[str, object], *path: str) -> List[str]:
    node = schema  # type: object
    for key in path:
        if not isinstance(node, dict) or key not in node:
            fail("Missing schema path: {}".format("/".join(path)))
        node = node[key]
    if not isinstance(node, list):
        fail("Schema path is not an enum list: {}".format("/".join(path)))
    return [str(value) for value in node]


def assert_unique(name: str, values: List[str]) -> None:
    seen = set()  # type: Set[str]
    duplicates = set()  # type: Set[str]
    for value in values:
        if value in seen:
            duplicates.add(value)
        seen.add(value)
    if duplicates:
        fail("{} contains duplicate values: {}".format(name, sorted(duplicates)))


def assert_schema_labels_are_known(
    name: str,
    labels: List[str],
    label_to_symbol: Dict[str, str],
) -> None:
    missing = [label for label in labels if label not in label_to_symbol]
    if missing:
        fail(
            "{} contains labels not accepted by ActionStringToEnum(): {}".format(
                name,
                missing,
            )
        )


def assert_contains_required_labels(
    name: str,
    labels: List[str],
    required_labels: List[str],
) -> None:
    missing = [label for label in required_labels if label not in labels]
    if missing:
        fail("{} is missing required labels: {}".format(name, missing))


def assert_config_round_trips(
    label_to_symbol: Dict[str, str],
    symbol_to_label: Dict[str, str],
) -> None:
    failures = []  # type: List[str]
    for label, symbol in sorted(label_to_symbol.items()):
        round_trip = symbol_to_label.get(symbol)
        if round_trip != label:
            failures.append("{!r} -> {} -> {!r}".format(label, symbol, round_trip))
    if failures:
        fail("Action string/enum round-trip failures: " + "; ".join(failures))


def web_option_labels_for_selected_expression(
    web_config: str,
    selected_expression: str,
) -> Set[str]:
    labels = set()  # type: Set[str]
    option_start = re.compile(r'page\s*\+=\s*F\("<option value=\'"\);')
    option_label = re.compile(r'page\s*\+=\s*F\(">([^"<]+)</option>"\);')

    for match in option_start.finditer(web_config):
        option_body = web_config[match.end() :]
        label_match = option_label.search(option_body)
        if not label_match:
            continue
        if selected_expression in option_body[: label_match.start()]:
            labels.add(label_match.group(1))

    if not labels:
        fail(
            "No WebConfigAsync.h option labels found for selected expression: {}".format(
                selected_expression,
            )
        )
    return labels


def assert_web_option_group_includes_required_labels(
    name: str,
    web_config: str,
    selected_expression: str,
    labels: List[str],
) -> None:
    options = web_option_labels_for_selected_expression(web_config, selected_expression)
    missing = [label for label in labels if label not in options]
    if missing:
        fail("WebConfigAsync.h {} is missing option labels: {}".format(name, missing))


def main() -> int:
    config_h = read_text(CONFIG_H)
    web_config = read_text(WEB_CONFIG)

    with SCHEMA_JSON.open("r", encoding="utf-8") as handle:
        schema = json.load(handle)

    label_to_symbol = action_string_to_enum_map(config_h)
    symbol_to_label = action_enum_to_string_map(config_h)
    assert_config_round_trips(label_to_symbol, symbol_to_label)

    action_messages = schema_enum(
        schema,
        "properties",
        "Actions",
        "items",
        "properties",
        "Message",
        "enum",
    )
    sequence_messages = schema_enum(
        schema,
        "properties",
        "Sequences",
        "items",
        "properties",
        "Message",
        "enum",
    )
    incoming_target_actions = schema_enum(
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

    assert_unique("Actions.Message schema enum", action_messages)
    assert_unique("Sequences.Message schema enum", sequence_messages)
    assert_unique("IncomingTriggers.Actions.Action schema enum", incoming_target_actions)
    assert_schema_labels_are_known(
        "Actions.Message schema enum",
        action_messages,
        label_to_symbol,
    )
    assert_schema_labels_are_known(
        "Sequences.Message schema enum",
        sequence_messages,
        label_to_symbol,
    )
    assert_schema_labels_are_known(
        "IncomingTriggers.Actions.Action schema enum",
        incoming_target_actions,
        label_to_symbol,
    )

    assert_contains_required_labels(
        "Actions.Message schema enum",
        action_messages,
        ["Sequence", "Set Led Color", "Set Slot State", "Repeat Overwrite"],
    )
    assert_contains_required_labels(
        "Sequences.Message schema enum",
        sequence_messages,
        ["Sequence", "Set Bank", "Set Led Color"],
    )
    assert_contains_required_labels(
        "IncomingTriggers.Actions.Action schema enum",
        incoming_target_actions,
        ["Set Led Color", "Set Slot State", "Set Bank"],
    )

    assert_web_option_group_includes_required_labels(
        "Actions.Message options",
        web_config,
        "act->midiMessage ==",
        ["Sequence", "Set Led Color", "Set Slot State", "Repeat Overwrite"],
    )
    assert_web_option_group_includes_required_labels(
        "Sequences.Message options",
        web_config,
        "sequences[s-1][i-1].midiMessage ==",
        ["Sequence", "Set Bank", "Set Led Color"],
    )
    assert_web_option_group_includes_required_labels(
        "IncomingTriggers.Actions.Action options",
        web_config,
        "bankTriggers[i].actions[a].targetAction ==",
        ["Set Led Color", "Set Slot State", "Set Bank"],
    )

    print("Config surface validation passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
