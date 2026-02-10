# PolarionMockup — Development Plan

## Overview

PolarionMockup is a lightweight C++ server that emulates the Siemens Polarion ALM REST API.
It enables **JarvisAgent** to develop and test Polarion integration features without requiring
access to a real Polarion server instance.

The mockup serves a JSON:API-compliant REST API (via Crow) that mirrors Polarion's
work-item and requirements-management endpoints. Data lives in-memory and is seeded at
startup.

**Namespace**: `PolarionMockup`

**Reference**: [Polarion REST API User Guide](https://developer.siemens.com/polarion/overview.html)
/ [OpenAPI Spec](https://developer.siemens.com/polarion/polarion-rest-apispec.json)

---

## JarvisAgent Interaction

PolarionMockup exists to serve as the backend for JarvisAgent's **Polarion REST API client**
(`PolarionClient` class in JarvisAgent).  The primary interaction is:

1. JarvisAgent defines **filter nodes** with `source.kind = "polarion_query"` in its
   JCWF workflow files.
2. At runtime, JarvisAgent's `PolarionClient` issues paginated `GET` requests:
   ```
   GET {base_url}/rest/v1/projects/{project_id}/workitems
     ?query={url_encoded_lucene_query}
     &fields[workitems]={comma_separated_fields}
     &page[size]=100
     &page[number]={N}
   ```
3. PolarionMockup authenticates via Bearer token, evaluates the Lucene-style query
   against its in-memory `DataStore`, and returns JSON:API responses.
4. JarvisAgent's **fan-out** mechanism then creates one task instance per returned
   work item, enabling per-item processing (summarization, analysis, etc.).

### Authentication

The mockup uses a **hard-coded Bearer token** for simplicity:
```
Token: 1234!@#$
Header: Authorization: Bearer 1234!@#$
```
The `Auth` class validates incoming requests against this static token.

---

## Architecture

```
polarionMockup/
├── src/             # All C++ source and headers
├── vendor/          # Third-party libs (Crow, spdlog, simdjson, openssl, …)
├── doc/             # Documentation and dev plan
├── bin/             # Build output (gitignored)
├── bin-int/         # Intermediate build files (gitignored)
└── premake5.lua     # Build configuration
```

---

## Coding Style

- **Allman** brace style
- **4-space** indentation, 125-column limit
- **Naming**: PascalCase classes/methods, `m_` member prefix, `s_` static prefix, `g_` global prefix
- **File names**: camelCase (e.g., `workItem.h`, `restServer.cpp`)
- **Header guards**: `#pragma once`
- **License header**: MIT — Copyright (c) 2026 JC Technolabs
- **Build system**: Premake5

---

## Polarion REST API — Interface Specification

The real Polarion REST API is based on the **JSON:API** standard with some deviations
(bulk create/update/delete). Base URL pattern: `{server}/polarion/rest/v1/`.

### Authentication

Polarion uses **Bearer token** authentication (Personal Access Token):
```
Authorization: Bearer 1234!@#$
```
The mockup validates against the hard-coded token `1234!@#$` (see `Auth` class).

### Core Resource Types

| Resource Type              | JSON:API `type`             | Description                          |
|----------------------------|-----------------------------|--------------------------------------|
| Projects                   | `projects`                  | Top-level project containers         |
| Work Items                 | `workitems`                 | Requirements, tasks, defects, etc.   |
| Documents                  | `documents`                 | Document containers for work items   |
| Collections                | `collections`               | Collections of documents             |
| Linked Work Items          | `linkedworkitems`           | Traceability links between items     |
| Work Item Comments         | `workitem_comments`         | Comments on work items               |
| Work Item Attachments      | `workitem_attachments`      | File attachments on work items       |
| Work Item Approvals        | `workitem_approvals`        | Approval workflow                    |
| Test Runs                  | `testruns`                  | Test execution runs                  |
| Test Records               | `testrecords`               | Individual test records              |
| Pages                      | `pages`                     | Rich/wiki pages in spaces            |
| Users                      | `users`                     | User accounts                        |
| Categories                 | `categories`                | Enumeration categories               |

### Work Item Attributes

| Attribute          | Type       | Example                        |
|--------------------|------------|--------------------------------|
| `id`               | string     | `"MyWorkItemId"`               |
| `type`             | string     | `"requirement"`, `"task"`      |
| `title`            | string     | `"Title"`                      |
| `description`      | rich text  | `{type: "text/html", value: "..."}` |
| `status`           | string     | `"open"`, `"closed"`          |
| `severity`         | string     | `"blocker"`, `"major"`        |
| `priority`         | string     | `"90.0"`                       |
| `resolution`       | string     | `"done"`                       |
| `created`          | date-time  | `"1970-01-01T00:00:00Z"`      |
| `updated`          | date-time  | `"1970-01-01T00:00:00Z"`      |
| `resolvedOn`       | date-time  | `"1970-01-01T00:00:00Z"`      |
| `dueDate`          | date       | `"1970-01-01"`                 |
| `plannedStart`     | date-time  | `"1970-01-01T00:00:00Z"`      |
| `plannedEnd`       | date-time  | `"1970-01-01T00:00:00Z"`      |
| `initialEstimate`  | string     | `"5 1/2d"`                     |
| `remainingEstimate`| string     | `"5 1/2d"`                     |
| `timeSpent`        | string     | `"5 1/2d"`                     |
| `outlineNumber`    | string     | `"1.11"`                       |
| `hyperlinks`       | array      | `[{role, title, uri}]`         |

### Work Item Relationships

| Relationship               | Target Type                | Cardinality |
|----------------------------|----------------------------|-------------|
| `author`                   | `users`                    | one         |
| `assignee`                 | `users`                    | many        |
| `categories`               | `categories`               | many        |
| `comments`                 | `workitem_comments`        | many        |
| `attachments`              | `workitem_attachments`     | many        |
| `approvals`                | `workitem_approvals`       | many        |
| `linkedWorkItems`          | `linkedworkitems`          | many        |
| `backlinkedWorkItems`      | `linkedworkitems`          | many        |
| `externallyLinkedWorkItems`| `externallylinkedworkitems`| many        |
| `linkedRevisions`          | `revisions`                | many        |
| `project`                  | `projects`                 | one         |
| `module`                   | `documents`                | one         |
| `votes`                    | `users`                    | many        |
| `watches`                  | `users`                    | many        |
| `workRecords`              | `workrecords`              | many        |

### Key Endpoint Patterns

All paths are prefixed with `/polarion/rest/v1`.

**Projects:**
```
GET    /projects
GET    /projects/{projectId}
```

**Work Items (core):**
```
GET    /projects/{projectId}/workitems                    — list/query (paginated)
POST   /projects/{projectId}/workitems                    — bulk create
PATCH  /projects/{projectId}/workitems                    — bulk update
DELETE /projects/{projectId}/workitems                    — bulk delete
GET    /projects/{projectId}/workitems/{workItemId}       — single get
PATCH  /projects/{projectId}/workitems/{workItemId}       — single update
DELETE /projects/{projectId}/workitems/{workItemId}       — single delete
```

**Work Item sub-resources:**
```
GET/POST/PATCH/DELETE .../workitems/{workItemId}/comments
GET/POST/PATCH/DELETE .../workitems/{workItemId}/attachments
GET/POST/DELETE       .../workitems/{workItemId}/approvals
GET/POST/DELETE       .../workitems/{workItemId}/linkedworkitems
POST                  .../workitems/{workItemId}/actions/moveToDocument
```

**Documents & Pages:**
```
GET/POST/PATCH/DELETE /projects/{projectId}/spaces/{spaceId}/documents
GET/POST/PATCH/DELETE /projects/{projectId}/spaces/{spaceId}/pages
```

**Collections:**
```
GET/POST/PATCH/DELETE /projects/{projectId}/collections
```

### Common Query Parameters

| Parameter       | Description                              |
|-----------------|------------------------------------------|
| `page[size]`    | Number of items per page                 |
| `page[number]`  | Page number (1-indexed)                  |
| `fields[type]`  | Sparse fieldsets per resource type        |
| `include`       | Include related resources (sideloading)  |
| `query`         | Polarion query language filter           |
| `sort`          | Sort by field(s), prefix `-` for desc    |
| `revision`      | Retrieve resource at specific revision   |

### JSON:API Response Envelope

```json
{
  "data": [ { "type": "workitems", "id": "...", "attributes": {...}, "relationships": {...}, "links": {...} } ],
  "included": [ ... ],
  "links": { "self": "...", "first": "...", "prev": "...", "next": "...", "last": "..." },
  "meta": { "totalCount": 42 }
}
```

---

## Milestones

### Phase 1 — Skeleton & Build

- [x] Bearer token authentication (`Auth` class, hard-coded token `1234!@#$`)
- [x] Premake5 builds cleanly on Linux (Debug + Release)
- [x] Entry point (`main.cpp`): spdlog init, `DataStore` construction, Crow server start
- [x] Crow HTTP server boots and responds to health-check (`GET /polarion/rest/v1/health`)
- [x] Auth middleware: reject requests without valid Bearer token

### Phase 2 — Data Model & Storage

- [x] `Resource` base class (type, id, revision)
- [x] `WorkItem` class matching Polarion attributes
- [x] `Project` class (id, name, description)
- [x] `Document` class (project, space, title, work-item list)
- [x] `User` class (id, name)
- [x] `RichText` / `Hyperlink` types
- [x] `DataStore` with in-memory maps and go-kart procurement seed data
- [x] `QueryParser` — Lucene-style AST (AND, OR, NOT, range, wildcard, grouping)
- [x] `QueryEngine` — evaluates query AST against work items
- [x] `JsonApiSerializer` — serialization for all resource types + envelopes
- [x] JSON:API deserialization (POST/PATCH request bodies in `main.cpp`)

### Phase 3 — Work Item REST API

- [x] `GET    /projects/{projectId}/workitems`          — list with pagination, query, sparse fields
- [x] `GET    /projects/{projectId}/workitems/{id}`     — single get
- [x] `POST   /projects/{projectId}/workitems`          — create (single + bulk)
- [x] `PATCH  /projects/{projectId}/workitems/{id}`     — update single
- [x] `PATCH  /projects/{projectId}/workitems`          — bulk update
- [x] `DELETE /projects/{projectId}/workitems/{id}`     — delete single
- [x] `DELETE /projects/{projectId}/workitems`          — bulk delete
- [x] JSON:API response envelope (data, links, meta)

### Phase 4 — Sub-Resources & Relationships

- [x] Linked work items (`GET/POST/DELETE .../workitems/{id}/linkedworkitems`)
- [x] Work item comments (`GET/POST/PATCH/DELETE .../workitems/{id}/comments`)
- [x] Work item attachments (`GET/POST/DELETE .../workitems/{id}/attachments`)
- [x] Documents & spaces (`GET /projects/{id}/spaces/{spaceId}/documents`)
- [x] Collections (`GET/POST/PATCH/DELETE /projects/{id}/collections`)

### Phase 5 — JarvisAgent Integration

- [x] Bearer token (PAT) authentication matches Polarion's `Authorization: Bearer <token>` scheme
- [x] Confirm JarvisAgent `PolarionClient` URL pattern matches mockup (`{base}/rest/v1/projects/{id}/workitems`)
- [x] Validate paginated responses work with JarvisAgent's `page[size]=100` pattern
- [x] Validate Lucene query pass-through: JarvisAgent filter → mockup query engine
- [x] Validate sparse fieldsets: `fields[workitems]=title,status` returns only requested attributes
- [ ] End-to-end test: JarvisAgent fan-out creates per-item task instances from mockup data

### Phase 6 — Optional Enhancements

- [ ] Simple browser UI for inspecting / editing work items
- [ ] Work-item history / revisions
- [ ] Polarion query language parser (basic subset)
- [ ] Project / module hierarchy

---

## Dependencies (vendor/)

| Library        | Purpose                        |
|----------------|--------------------------------|
| Crow + Asio    | HTTP/WebSocket server          |
| spdlog         | Logging                        |
| simdjson       | Fast JSON parsing              |
| OpenSSL        | TLS / HTTPS support            |

---

## Build & Run

```bash
premake5 gmake
export MAKEFLAGS=-j$(nproc)
make config=release
./bin/Release/mockup
```
